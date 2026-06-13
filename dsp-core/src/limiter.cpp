#include "stages.h"
#include <cmath>
#include <algorithm>

namespace af {

static const float PI = 3.14159265358979f;
static inline float sinc(float x) {
    if (std::fabs(x) < 1e-6f) return 1.0f;
    return std::sin(PI * x) / (PI * x);
}

void Limiter::configure(float fs, const AfParams& p) {
    ceiling_ = std::pow(10.0f, p.limiter_ceiling_db / 20.0f);
    la_ = std::max(1, (int)std::lround(0.002f * fs)); // 2 ms de look-ahead
    atk_ = std::exp(-1.0f / std::max(1.0f, (float)la_));
    rel_ = std::exp(-1.0f / std::max(1.0f, p.limiter_release_ms * 0.001f * fs));
    for (int c = 0; c < 2; ++c) {
        dl_[c].assign(la_, 0.0f);
        hist_[c].assign(TPT, 0.0f);
    }
    widx_ = 0; hpos_ = 0; gain_ = 1.0f;

    // FIR de interpolação 4x (sinc janelada de Hann), centrada no meio do histórico.
    for (int ph = 0; ph < 4; ++ph) {
        float pos = (TPT / 2 - 1) + ph / 4.0f, sum = 0.0f;
        for (int j = 0; j < TPT; ++j) {
            float w = 0.5f - 0.5f * std::cos(2.0f * PI * j / (TPT - 1));
            coef_[ph][j] = sinc(pos - j) * w;
            sum += coef_[ph][j];
        }
        for (int j = 0; j < TPT; ++j) coef_[ph][j] /= sum; // ganho unitário (DC)
    }
}

void Limiter::reset() {
    for (int c = 0; c < 2; ++c) {
        std::fill(dl_[c].begin(), dl_[c].end(), 0.0f);
        std::fill(hist_[c].begin(), hist_[c].end(), 0.0f);
    }
    widx_ = 0; hpos_ = 0; gain_ = 1.0f;
}

// Maior magnitude inter-amostra perto da amostra recém-inserida (em hist_).
float Limiter::truePeak(int ch) {
    float tp = 0.0f;
    for (int c = 0; c < ch; ++c) {
        const float* h = hist_[c].data();
        for (int ph = 0; ph < 4; ++ph) {
            float acc = 0.0f;
            for (int j = 0; j < TPT; ++j) acc += coef_[ph][j] * h[(hpos_ + j) % TPT];
            tp = std::fmax(tp, std::fabs(acc));
        }
    }
    return tp;
}

void Limiter::process(float* L, float* R, int n, int ch) {
    for (int i = 0; i < n; ++i) {
        float inL = L[i];
        float inR = (ch == 2) ? R[i] : 0.0f;

        // empurra para o histórico de superamostragem (ring; mais antigo em hpos_)
        hist_[0][hpos_] = inL;
        if (ch == 2) hist_[1][hpos_] = inR;
        hpos_ = (hpos_ + 1) % TPT;

        // amostra que sai do delay (atrasada em la_)
        float outL = dl_[0][widx_];
        float outR = (ch == 2) ? dl_[1][widx_] : 0.0f;
        dl_[0][widx_] = inL;
        if (ch == 2) dl_[1][widx_] = inR;
        widx_ = (widx_ + 1) % la_;

        // ganho alvo a partir do TRUE-PEAK (inclui picos inter-amostra).
        float peak = truePeak(ch);
        float target = (peak > ceiling_) ? (ceiling_ / peak) : 1.0f;
        if (target < gain_) gain_ = atk_ * gain_ + (1.0f - atk_) * target;
        else                gain_ = rel_ * gain_ + (1.0f - rel_) * target;

        outL *= gain_;
        outR *= gain_;

        // rede de segurança (domínio de amostra)
        outL = std::max(-ceiling_, std::min(ceiling_, outL));
        L[i] = outL;
        if (ch == 2) {
            outR = std::max(-ceiling_, std::min(ceiling_, outR));
            R[i] = outR;
        }
    }
}

} // namespace af
