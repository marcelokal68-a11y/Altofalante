#include "stages.h"
#include <cmath>

namespace af {

void Exciter::configure(float fs, const AfParams& p) {
    drive_ = p.exciter_drive;
    mix_   = p.exciter_mix;
    for (int c = 0; c < 2; ++c) {
        lp_[c].setLowpass(fs, p.exciter_freq_hz, 0.707f);
        // Mantem apenas os harmonicos gerados (acima da banda do fundamental).
        hp_[c].setHighpass(fs, p.exciter_freq_hz, 0.707f);
    }
}

void Exciter::reset() {
    for (int c = 0; c < 2; ++c) { lp_[c].reset(); hp_[c].reset(); }
}

// Gera harmonicos a partir do grave: isola graves -> nao-linearidade -> mantem
// harmonicos -> mistura com o seco. O ouvido reconstroi o fundamental ausente.
void Exciter::process(float* L, float* R, int n, int ch) {
    if (mix_ <= 0.0f || drive_ <= 0.0f) return;
    for (int i = 0; i < n; ++i) {
        float low = lp_[0].process(L[i]);
        float harm = std::tanh(drive_ * 4.0f * low);   // gera harmonicos
        harm = hp_[0].process(harm);                    // remove o fundamental
        L[i] += mix_ * harm;
        if (ch == 2) {
            float lowR = lp_[1].process(R[i]);
            float harmR = std::tanh(drive_ * 4.0f * lowR);
            harmR = hp_[1].process(harmR);
            R[i] += mix_ * harmR;
        }
    }
}

} // namespace af
