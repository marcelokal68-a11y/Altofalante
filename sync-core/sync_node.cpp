/*
 * sync_node — CLI fino sobre a API C do sync (altofalante/sync_api.h). Prova a mesma
 * API que a ponte nativa do app usa. Mede a sincronizacao real via CLOCK_MONOTONIC.
 *
 * Papeis:
 *   leader   <expectedFollowers> <theta>
 *   follower <theta> <latency> <resultFile> <K(ignorado)>
 *
 * AF_IFACE (env) escolhe a interface multicast (padrao loopback; use o IP local da
 * maquina para testar entre 2 computadores na mesma Wi-Fi).
 */
#include "altofalante/sync_api.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>

static int runLeader(int expected, double theta) {
    AfSync* s = af_sync_create(theta, nullptr);
    if (af_sync_start_leader(s) != 0) { fprintf(stderr, "leader: falha\n"); return 1; }
    printf("leader: anunciando (theta=%.3f), esperando %d seguidores...\n", theta, expected);

    // Espera todos registrarem E terminarem a sondagem (>1.2s) antes do PLAY.
    // (No app, o play acontece muito depois do "entrar", entao nao ha essa corrida.)
    double start = af_sync_now();
    while ((af_sync_now() - start) < 3.0) {
        if (af_sync_follower_count(s) >= expected && (af_sync_now() - start) > 1.2) break;
        usleep(50000);
    }

    double fire = af_sync_leader_play(s, 0.8, 0.0); // latencia 0 no teste
    printf("leader: PLAY enviado (%d seguidores), inicio monotonico=%.6f\n",
           af_sync_follower_count(s), fire);
    usleep(2000000);
    af_sync_destroy(s);
    return 0;
}

static int runFollower(double theta, double latency, const std::string& resultFile) {
    AfSync* s = af_sync_create(theta, nullptr);
    if (af_sync_join(s, 4000) != 0) { fprintf(stderr, "follower: nao achou o lider\n"); return 2; }

    char ip[64] = {0};
    int port = af_sync_leader_endpoint(s, ip, sizeof(ip));
    printf("follower theta=%+.3f: descobriu lider em %s:%d (offset=%.6f)\n",
           theta, ip, port, af_sync_offset(s));

    double monoFire = af_sync_wait_play(s, latency, 5000);
    if (monoFire < 0) { fprintf(stderr, "follower: sem PLAY\n"); af_sync_destroy(s); return 3; }

    while (af_sync_now() < monoFire) {
        double rem = monoFire - af_sync_now();
        if (rem > 0.002) usleep((useconds_t)((rem - 0.001) * 1e6));
    }
    double emit = af_sync_now() + latency;

    FILE* f = fopen(resultFile.c_str(), "a");
    if (f) { fprintf(f, "%.9f\n", emit); fclose(f); }
    printf("follower theta=%+.3f lat=%.3f emit=%.9f\n", theta, latency, emit);
    af_sync_destroy(s);
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "uso: sync_node leader|follower ...\n"); return 1; }
    std::string role = argv[1];
    if (role == "leader" && argc >= 4)
        return runLeader(atoi(argv[2]), atof(argv[3]));
    if (role == "follower" && argc >= 6)
        return runFollower(atof(argv[2]), atof(argv[3]), argv[4]);
    fprintf(stderr,
            "uso:\n  sync_node leader <expected> <theta>\n"
            "  sync_node follower <theta> <latency> <file> <K>\n");
    return 1;
}
