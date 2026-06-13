#include "stages.h"
#include <cmath>
#include <algorithm>

namespace af {

void Limiter::configure(float fs, const AfParams& p) {
    ceiling_ = std::pow(10.0f, p.limiter_ceiling_db / 20.0f);
    la_ = std::max(1, (int)std::lround(0.002f * fs)); // 2 ms de look-ahead
    // Attack acompanha o look-ahead; release configuravel.
    atk_ = std::exp(-1.0f / std::max(1.0f, (float)la_));
    rel_ = std::exp(-1.0f / std::max(1.0f, p.limiter_release_ms * 0.001f * fs));
    for (int c = 0; c < 2; ++c) dl_[c].assign(la_, 0.0f);
    widx_ = 0;
    gain_ = 1.0f;
}

void Limiter::reset() {
    for (int c = 0; c < 2; ++c) std::fill(dl_[c].begin(), dl_[c].end(), 0.0f);
    widx_ = 0;
    gain_ = 1.0f;
}

void Limiter::process(float* L, float* R, int n, int ch) {
    for (int i = 0; i < n; ++i) {
        float inL = L[i];
        float inR = (ch == 2) ? R[i] : 0.0f;

        // Amostra que esta saindo do delay (atrasada em la_ amostras).
        float outL = dl_[0][widx_];
        float outR = (ch == 2) ? dl_[1][widx_] : 0.0f;
        dl_[0][widx_] = inL;
        if (ch == 2) dl_[1][widx_] = inR;
        widx_ = (widx_ + 1) % la_;

        // Ganho alvo a partir do pico que esta CHEGANDO (look-ahead).
        float peak = std::max(std::fabs(inL), std::fabs(inR));
        float target = (peak > ceiling_) ? (ceiling_ / peak) : 1.0f;

        // Desce rapido (attack), recupera devagar (release).
        if (target < gain_) gain_ = atk_ * gain_ + (1.0f - atk_) * target;
        else                gain_ = rel_ * gain_ + (1.0f - rel_) * target;

        outL *= gain_;
        outR *= gain_;

        // Rede de seguranca: clip absoluto no teto (garante INV1).
        outL = std::max(-ceiling_, std::min(ceiling_, outL));
        L[i] = outL;
        if (ch == 2) {
            outR = std::max(-ceiling_, std::min(ceiling_, outR));
            R[i] = outR;
        }
    }
}

} // namespace af
