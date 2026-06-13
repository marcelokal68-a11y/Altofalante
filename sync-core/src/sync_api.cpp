#include "altofalante/sync_api.h"
#include "altofalante/sync.h"
#include "altofalante/net.h"
#include "protocol.h"

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>

using namespace af;
using namespace af::net;

namespace {
std::string resolveIface(const char* iface) {
    if (iface && *iface) return iface;
    const char* e = getenv("AF_IFACE");
    return (e && *e) ? std::string(e) : std::string(MCAST_IFACE);
}
} // namespace

struct AfSync {
    double theta = 0;
    std::string iface;

    // Líder.
    std::thread th;
    std::atomic<bool> running{false};
    UdpSocket data, ann;
    std::mutex mtx;
    std::vector<Addr> followers;

    // Seguidor.
    UdpSocket fsock;
    Addr leaderAddr{};
    double offset = 0;

    double clk() const { return monotonicNow() + theta; }

    void serveLoop() {
        Addr group = UdpSocket::makeAddr(MCAST_GROUP, MCAST_PORT);
        uint16_t dataPort = data.localPort();
        double lastAnn = 0;
        while (running.load()) {
            double nowm = monotonicNow();
            if (nowm - lastAnn > 0.2) {
                Packet a{}; a.type = MSG_ANNOUNCE; a.a = (double)dataPort;
                ann.sendTo(&a, sizeof(a), group);
                lastAnn = nowm;
            }
            Packet p; Addr from;
            if (data.recv(&p, sizeof(p), from, 50) == (int)sizeof(p)) {
                if (p.type == MSG_HELLO) {
                    std::lock_guard<std::mutex> lk(mtx);
                    bool known = false;
                    for (auto& f : followers) if (f == from) known = true;
                    if (!known) followers.push_back(from);
                } else if (p.type == MSG_PROBE) {
                    Packet r{}; r.type = MSG_PROBE_REPLY; r.seq = p.seq;
                    r.a = p.a; r.b = clk(); r.c = clk();
                    data.sendTo(&r, sizeof(r), from);
                }
            }
        }
    }
};

extern "C" {

AfSync* af_sync_create(double theta, const char* iface) {
    auto* s = new AfSync();
    s->theta = theta;
    s->iface = resolveIface(iface);
    return s;
}

void af_sync_destroy(AfSync* s) {
    if (!s) return;
    s->running = false;
    if (s->th.joinable()) s->th.join();
    delete s;
}

double af_sync_now(void) { return monotonicNow(); }

int af_sync_start_leader(AfSync* s) {
    if (!s->data.open() || !s->data.bindPort(0)) return -1;
    if (!s->ann.open()) return -1;
    s->ann.setMulticastIface(s->iface);
    s->ann.enableMulticastLoop();
    s->running = true;
    s->th = std::thread([s]() { s->serveLoop(); });
    return 0;
}

int af_sync_follower_count(AfSync* s) {
    std::lock_guard<std::mutex> lk(s->mtx);
    return (int)s->followers.size();
}

double af_sync_leader_play(AfSync* s, double leadSeconds, double outputLatency) {
    double leaderStart = s->clk() + leadSeconds;
    Packet pl{}; pl.type = MSG_PLAY; pl.a = leaderStart;
    {
        std::lock_guard<std::mutex> lk(s->mtx);
        for (auto& f : s->followers) s->data.sendTo(&pl, sizeof(pl), f);
    }
    return (leaderStart - s->theta) - outputLatency; // instante monotônico p/ iniciar
}

int af_sync_join(AfSync* s, int timeoutMs) {
    UdpSocket disc;
    if (!disc.open() || !disc.joinMulticast(MCAST_GROUP, MCAST_PORT, s->iface)) return -1;
    bool found = false;
    double t0 = monotonicNow();
    while ((monotonicNow() - t0) * 1000.0 < timeoutMs) {
        Packet a; Addr from;
        if (disc.recv(&a, sizeof(a), from, 300) == (int)sizeof(a) &&
            a.type == MSG_ANNOUNCE) {
            s->leaderAddr = from;
            s->leaderAddr.sa.sin_port = htons((uint16_t)a.a);
            found = true; break;
        }
    }
    if (!found) return -1;

    if (!s->fsock.open() || !s->fsock.bindPort(0)) return -1;
    Packet hello{}; hello.type = MSG_HELLO;
    s->fsock.sendTo(&hello, sizeof(hello), s->leaderAddr);

    std::vector<sync::Sample> samples;
    for (int k = 0; k < 20; ++k) {
        Packet p{}; p.type = MSG_PROBE; p.seq = k; p.a = s->clk();
        s->fsock.sendTo(&p, sizeof(p), s->leaderAddr);
        Packet r; Addr from;
        if (s->fsock.recv(&r, sizeof(r), from, 200) == (int)sizeof(r) &&
            r.type == MSG_PROBE_REPLY) {
            samples.push_back({r.a, r.b, r.c, s->clk()});
        }
        usleep(5000);
    }
    if (samples.empty()) return -1;
    s->offset = sync::estimateOffset(samples).offset;
    return 0;
}

int af_sync_leader_endpoint(AfSync* s, char* ipOut, int ipLen) {
    if (s->leaderAddr.sa.sin_port == 0) return -1;
    if (ipOut && ipLen > 0)
        inet_ntop(AF_INET, &s->leaderAddr.sa.sin_addr, ipOut, ipLen);
    return ntohs(s->leaderAddr.sa.sin_port);
}

double af_sync_offset(AfSync* s) { return s->offset; }

double af_sync_wait_play(AfSync* s, double outputLatency, int timeoutMs) {
    double t0 = monotonicNow();
    while ((monotonicNow() - t0) * 1000.0 < timeoutMs) {
        Packet r; Addr from;
        if (s->fsock.recv(&r, sizeof(r), from, 200) == (int)sizeof(r) &&
            r.type == MSG_PLAY) {
            double localStart = sync::followerLocalStart(r.a, s->offset, outputLatency);
            return localStart - s->theta; // instante monotônico p/ iniciar
        }
    }
    return -1.0;
}

} // extern "C"
