#include "altofalante/sync.h"

namespace af::sync {

// O relogio local do seguidor no instante de parede de `leaderStart` e
// (leaderStart - offset). Para o som emergir nesse instante, o seguidor precisa
// iniciar a reproducao com antecedencia igual a sua latencia de saida.
double followerLocalStart(double leaderStart, double offset, double outputLatency) {
    return (leaderStart - offset) - outputLatency;
}

} // namespace af::sync
