/*
 * sync_node — no real de sincronizacao multi-celular sobre UDP.
 *
 * Papeis:
 *   leader   <port> <expectedFollowers> <theta>
 *   follower <host> <port> <theta> <latency> <resultFile> <K>
 *
 * `theta`  = offset artificial do relogio do aparelho (simula relogios fora de
 *            sincronia entre celulares diferentes).
 * `latency`= latencia de saida de audio do aparelho (compensada no agendamento).
 *
 * Mede a sincronizacao REAL: todos compartilham o CLOCK_MONOTONIC (mesma maquina),
 * entao o instante "emit" reportado por cada seguidor e comparavel. A diferenca
 * entre os emit dos seguidores e a defasagem real apos sincronizar pela rede.
 */
#include "altofalante/sync.h"
#include "altofalante/net.h"
#include "protocol.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <unistd.h>

using namespace af;
using namespace af::net;

static int runLeader(uint16_t port, int expected, double theta) {
    UdpSocket s;
    if (!s.open() || !s.bindPort(port)) { perror("leader bind"); return 1; }
    printf("leader: ouvindo na porta %u (theta=%.3f)\n", port, theta);

    std::vector<Addr> followers;
    double start = monotonicNow();
    bool played = false;

    while (true) {
        Packet p; Addr from;
        int n = s.recv(&p, sizeof(p), from, 50);
        if (n == (int)sizeof(p)) {
            if (p.type == MSG_HELLO) {
                bool known = false;
                for (auto& f : followers) if (f == from) known = true;
                if (!known) followers.push_back(from);
            } else if (p.type == MSG_PROBE) {
                Packet r{};
                r.type = MSG_PROBE_REPLY; r.seq = p.seq;
                r.a = p.a;                       // t0 (relogio do seguidor)
                r.b = monotonicNow() + theta;    // t1 (recebido, relogio do lider)
                r.c = monotonicNow() + theta;    // t2 (respondido)
                s.sendTo(&r, sizeof(r), from);
            }
        }
        double elapsed = monotonicNow() - start;
        bool ready = (int)followers.size() >= expected && elapsed > 1.5;
        if (!played && (ready || elapsed > 2.5) && !followers.empty()) {
            double leaderStart = (monotonicNow() + theta) + 0.8; // 0.8s no futuro
            Packet pl{}; pl.type = MSG_PLAY; pl.a = leaderStart;
            for (auto& f : followers) s.sendTo(&pl, sizeof(pl), f);
            played = true;
            printf("leader: PLAY enviado (leaderStart=%.6f, seguidores=%zu)\n",
                   leaderStart, followers.size());
        }
        if (played && elapsed > 4.0) break;
    }
    return 0;
}

static int runFollower(const std::string& host, uint16_t port, double theta,
                       double latency, const std::string& resultFile, int K) {
    UdpSocket s;
    if (!s.open() || !s.bindPort(0)) { perror("follower bind"); return 1; }
    Addr leader = UdpSocket::makeAddr(host, port);
    auto clk = [&]() { return monotonicNow() + theta; };

    Packet hello{}; hello.type = MSG_HELLO;
    s.sendTo(&hello, sizeof(hello), leader);

    // Sondagem de relogio (NTP-like).
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

    // Aguarda PLAY.
    double leaderStart = 0; bool got = false;
    double t0 = monotonicNow();
    while (monotonicNow() - t0 < 5.0) {
        Packet r; Addr from;
        if (s.recv(&r, sizeof(r), from, 200) == (int)sizeof(r) && r.type == MSG_PLAY) {
            leaderStart = r.a; got = true; break;
        }
    }
    if (!got) { fprintf(stderr, "follower: sem PLAY\n"); return 3; }

    // Agenda inicio local e espera com precisao.
    double localStart = sync::followerLocalStart(leaderStart, est.offset, latency);
    double monoFire = localStart - theta; // relogio local -> monotonico compartilhado
    while (true) {
        double now = monotonicNow();
        if (now >= monoFire) break;
        double rem = monoFire - now;
        if (rem > 0.002) usleep((useconds_t)((rem - 0.001) * 1e6));
    }
    double emit = monotonicNow() + latency; // instante em que o som emerge

    FILE* f = fopen(resultFile.c_str(), "a");
    if (f) { fprintf(f, "%.9f\n", emit); fclose(f); }
    printf("follower theta=%+.3f lat=%.3f offsetEst=%.6f emit=%.9f\n",
           theta, latency, est.offset, emit);
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "uso: sync_node leader|follower ...\n"); return 1; }
    std::string role = argv[1];
    if (role == "leader" && argc >= 5) {
        return runLeader((uint16_t)atoi(argv[2]), atoi(argv[3]), atof(argv[4]));
    }
    if (role == "follower" && argc >= 8) {
        return runFollower(argv[2], (uint16_t)atoi(argv[3]), atof(argv[4]),
                           atof(argv[5]), argv[6], atoi(argv[7]));
    }
    fprintf(stderr,
            "uso:\n  sync_node leader <port> <expected> <theta>\n"
            "  sync_node follower <host> <port> <theta> <latency> <file> <K>\n");
    return 1;
}
