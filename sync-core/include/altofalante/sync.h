/*
 * Altofalante sync-core — sincronizacao de relogio para tocar em varios celulares.
 *
 * Estima o offset entre o relogio do aparelho seguidor e o do lider (estilo NTP,
 * com filtro de menor atraso), para que todos emitam o audio no mesmo instante.
 */
#ifndef ALTOFALANTE_SYNC_H
#define ALTOFALANTE_SYNC_H

#include <vector>

namespace af::sync {

// Uma troca de timestamps (NTP de 4 marcas), em segundos.
//  t0: seguidor envia (relogio do seguidor)
//  t1: lider recebe  (relogio do lider)
//  t2: lider responde(relogio do lider)
//  t3: seguidor recebe(relogio do seguidor)
struct Sample { double t0, t1, t2, t3; };

struct OffsetEstimate {
    double offset; // (relogio_lider - relogio_seguidor), segundos
    double delay;  // round-trip estimado da melhor amostra, segundos
};

// Estima o offset usando a amostra de menor round-trip (a com menos fila/assimetria).
OffsetEstimate estimateOffset(const std::vector<Sample>& samples);

// Instante (relogio LOCAL do seguidor) em que o seguidor deve INICIAR a reproducao
// para que o som emerja no instante de parede correspondente a `leaderStart`.
//  offset = (relogio_lider - relogio_seguidor) estimado
//  outputLatency = latencia de saida de audio do aparelho (sa segundos)
double followerLocalStart(double leaderStart, double offset, double outputLatency);

} // namespace af::sync

#endif
