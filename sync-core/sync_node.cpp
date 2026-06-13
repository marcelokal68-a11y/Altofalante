/*
 * sync_node — no real de sincronizacao multi-celular sobre UDP, com DESCOBERTA.
 *
 * Papeis:
 *   leader   <expectedFollowers> <theta>
 *   follower <theta> <latency> <resultFile> <K>
 *
 * O lider anuncia sua presenca por multicast (estilo mDNS); o seguidor descobre o
 * endereco do lider sozinho (sem IP fixo), entao sincroniza o relogio e agenda o
 * inicio. Mede a sincronizacao real via CLOCK_MONOTONIC compartilhado.
 */
#include "altofalante/sync.h"
#include "altofalante/net.h"
#include "protocol.h"

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>

using namespace af;
using namespace af::net;

// Interface de rede usada para o multicast de descoberta.
// Padrao: loopback (testa tudo numa maquina). Para testar entre 2 computadores na
// mesma Wi-Fi, defina AF_IFACE com o IP local da maquina, ex.: AF_IFACE=192.168.0.10
static std::string iface() {
    const char* e = getenv("AF_IFACE");
    return (e && *e) ? std::string(e) : std::string(MCAST_IFACE);
}

static int runLeader(int expected, double theta) {
    UdpSocket data, ann;
    if (!data.open() || !data.bindPort(0)) { perror("leader data"); return 1; }
    if (!ann.open()) { perror("leader ann"); return 1; }
    ann.setMulticastIface(iface());
    ann.enableMulticastLoop();
    uint16_t dataPort = data.localPort();
    Addr group = UdpSocket::makeAddr(MCAST_GROUP, MCAST_PORT);
    printf("leader: porta de dados %u, anunciando em %s:%u (theta=%.3f)\n",
           dataPort, MCAST_GROUP, MCAST_PORT, theta);

    std::vector<Addr> followers;
    double start = monotonicNow(), lastAnn = 0;
    bool played = false;

    while (true) {
        double nowm = monotonicNow();
        if (nowm - lastAnn > 0.2) { // anuncia a cada 200 ms
            Packet a{}; a.type = MSG_ANNOUNCE; a.a = (double)dataPort;
            ann.sendTo(&a, sizeof(a), group);
            lastAnn = nowm;
        }
        Packet p; Addr from;
        if (data.recv(&p, sizeof(p), from, 50) == (int)sizeof(p)) {
            if (p.type == MSG_HELLO) {
                bool known = false;
                for (auto& f : followers) if (f == from) known = true;
                if (!known) followers.push_back(from);
            } else if (p.type == MSG_PROBE) {
                Packet r{}; r.type = MSG_PROBE_REPLY; r.seq = p.seq;
                r.a = p.a;
                r.b = monotonicNow() + theta;
                r.c = monotonicNow() + theta;
                data.sendTo(&r, sizeof(r), from);
            }
        }
        double elapsed = monotonicNow() - start;
        bool ready = (int)followers.size() >= expected && elapsed > 1.5;
        if (!played && (ready || elapsed > 2.5) && !followers.empty()) {
            double leaderStart = (monotonicNow() + theta) + 0.8;
            Packet pl{}; pl.type = MSG_PLAY; pl.a = leaderStart;
            for (auto& f : followers) data.sendTo(&pl, sizeof(pl), f);
            played = true;
            printf("leader: PLAY enviado (leaderStart=%.6f, seguidores=%zu)\n",
                   leaderStart, followers.size());
        }
        if (played && elapsed > 4.0) break;
    }
    return 0;
}

static int runFollower(double theta, double latency,
                       const std::string& resultFile, int K) {
    // --- descoberta: ouve anuncios multicast do lider ---
    UdpSocket disc;
    if (!disc.open() || !disc.joinMulticast(MCAST_GROUP, MCAST_PORT, iface())) {
        perror("follower multicast"); return 1;
    }
    Addr leader{}; bool found = false;
    double t0 = monotonicNow();
    while (monotonicNow() - t0 < 4.0) {
        Packet a; Addr from;
        if (disc.recv(&a, sizeof(a), from, 300) == (int)sizeof(a) &&
            a.type == MSG_ANNOUNCE) {
            leader = from;
            leader.sa.sin_port = htons((uint16_t)a.a); // porta de dados anunciada
            found = true;
            char ip[32]; inet_ntop(AF_INET, &from.sa.sin_addr, ip, sizeof(ip));
            printf("follower theta=%+.3f: descobriu lider em %s:%u\n",
                   theta, ip, (uint16_t)a.a);
            break;
        }
    }
    if (!found) { fprintf(stderr, "follower: nao descobriu o lider\n"); return 2; }

    UdpSocket s;
    if (!s.open() || !s.bindPort(0)) { perror("follower data"); return 1; }
    auto clk = [&]() { return monotonicNow() + theta; };

    Packet hello{}; hello.type = MSG_HELLO;
    s.sendTo(&hello, sizeof(hello), leader);

    std::vector<sync::Sample> samples;
    for (int k = 0; k < K; ++k) {
        Packet p{}; p.type = MSG_PROBE; p.seq = k; p.a = clk();
        s.sendTo(&p, sizeof(p), leader);
        Packet r; Addr from;
        if (s.recv(&r, sizeof(r), from, 200) == (int)sizeof(r) &&
            r.type == MSG_PROBE_REPLY) {
            samples.push_back({r.a, r.b, r.c, clk()});
        }
        usleep(5000);
    }
    if (samples.empty()) { fprintf(stderr, "follower: sem respostas\n"); return 2; }
    sync::OffsetEstimate est = sync::estimateOffset(samples);

    double leaderStart = 0; bool got = false;
    double tw = monotonicNow();
    while (monotonicNow() - tw < 5.0) {
        Packet r; Addr from;
        if (s.recv(&r, sizeof(r), from, 200) == (int)sizeof(r) && r.type == MSG_PLAY) {
            leaderStart = r.a; got = true; break;
        }
    }
    if (!got) { fprintf(stderr, "follower: sem PLAY\n"); return 3; }

    double localStart = sync::followerLocalStart(leaderStart, est.offset, latency);
    double monoFire = localStart - theta;
    while (true) {
        double now = monotonicNow();
        if (now >= monoFire) break;
        double rem = monoFire - now;
        if (rem > 0.002) usleep((useconds_t)((rem - 0.001) * 1e6));
    }
    double emit = monotonicNow() + latency;

    FILE* f = fopen(resultFile.c_str(), "a");
    if (f) { fprintf(f, "%.9f\n", emit); fclose(f); }
    printf("follower theta=%+.3f lat=%.3f offsetEst=%.6f emit=%.9f\n",
           theta, latency, est.offset, emit);
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "uso: sync_node leader|follower ...\n"); return 1; }
    std::string role = argv[1];
    if (role == "leader" && argc >= 4) {
        return runLeader(atoi(argv[2]), atof(argv[3]));
    }
    if (role == "follower" && argc >= 6) {
        return runFollower(atof(argv[2]), atof(argv[3]), argv[4], atoi(argv[5]));
    }
    fprintf(stderr,
            "uso:\n  sync_node leader <expected> <theta>\n"
            "  sync_node follower <theta> <latency> <file> <K>\n");
    return 1;
}
