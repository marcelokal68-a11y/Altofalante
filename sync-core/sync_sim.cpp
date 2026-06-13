/*
 * sync_sim — prova que 4 celulares conseguem tocar em sincronia.
 *
 * Modela: relogios bem dessincronizados, jitter de rede (assimetrico) nas trocas
 * NTP, e latencias de saida de audio diferentes por aparelho. Estima o offset e
 * agenda o inicio; mede a defasagem residual entre os alto-falantes.
 * Meta (specs/04): defasagem < 10 ms. Determinista (seed fixa) para CI.
 */
#include "altofalante/sync.h"
#include <cstdio>
#include <cmath>
#include <random>
#include <vector>
#include <algorithm>

using namespace af::sync;

int main() {
    std::mt19937 rng(12345);
    std::exponential_distribution<double> jitter(1.0 / 0.002); // media 2 ms
    std::uniform_real_distribution<double> uoff(-50.0, 50.0);  // relogios ate +-50 s
    std::uniform_real_distribution<double> ulat(0.020, 0.060); // latencia 20-60 ms
    std::uniform_real_distribution<double> ulaterr(-0.002, 0.002); // erro de calib +-2 ms

    const int N = 4;            // celulares seguidores
    const int K = 40;           // trocas NTP por aparelho
    const double base = 0.0015; // atraso base de uma via (1.5 ms)
    const double leaderProc = 0.0002;
    const double theta_leader = 0.0;
    const double T_start = 10.0; // instante de inicio no relogio do lider

    std::vector<double> emergence(N);
    printf("%-8s %12s %12s %12s\n", "device", "clkOffset(s)", "estErr(ms)", "emergErr(ms)");
    printf("------------------------------------------------------------\n");

    for (int i = 0; i < N; ++i) {
        double theta = uoff(rng);          // offset real do relogio do aparelho
        double Ltrue = ulat(rng);          // latencia real de saida
        double Lassumed = Ltrue + ulaterr(rng);

        std::vector<Sample> samples;
        double w0 = 0.0;
        for (int k = 0; k < K; ++k) {
            double dup = base + jitter(rng);
            double ddn = base + jitter(rng);
            double w1 = w0 + dup;
            double w2 = w1 + leaderProc;
            double w3 = w2 + ddn;
            Sample s;
            s.t0 = w0 + theta;          // relogio do seguidor
            s.t1 = w1 + theta_leader;   // relogio do lider
            s.t2 = w2 + theta_leader;
            s.t3 = w3 + theta;
            samples.push_back(s);
            w0 += 0.1; // proxima troca 100 ms depois
        }

        OffsetEstimate est = estimateOffset(samples);
        double trueOffset = theta_leader - theta;       // (lider - seguidor)
        double estErr = est.offset - trueOffset;

        // Agenda inicio local e calcula quando o som realmente emerge (parede).
        double Slocal = followerLocalStart(T_start, est.offset, Lassumed);
        double issueWall = Slocal - theta;              // relogio local -> parede
        emergence[i] = issueWall + Ltrue;               // som sai do alto-falante

        double W = T_start - theta_leader;              // instante de parede alvo
        printf("%-8d %12.3f %12.3f %12.3f\n",
               i, theta, estErr * 1000.0, (emergence[i] - W) * 1000.0);
    }

    double mx = *std::max_element(emergence.begin(), emergence.end());
    double mn = *std::min_element(emergence.begin(), emergence.end());
    double skew_ms = (mx - mn) * 1000.0;

    printf("------------------------------------------------------------\n");
    printf("Defasagem maxima entre aparelhos: %.3f ms (meta < 10 ms)\n", skew_ms);
    bool ok = skew_ms < 10.0;
    printf("%s\n", ok ? "SYNC OK" : "SYNC FALHOU");
    return ok ? 0 : 1;
}
