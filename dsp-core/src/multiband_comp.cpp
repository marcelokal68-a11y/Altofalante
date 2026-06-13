#include "stages.h"
#include <cmath>
#include <algorithm>

namespace af {

static inline float msToCoeff(float ms, float fs) {
    if (ms <= 0.0f) return 0.0f;
    return std::exp(-1.0f / (ms * 0.001f * fs));
}

void MultibandComp::configure(float fs, const AfParams& p) {
    fs_ = fs;
    for (int c = 0; c < 2; ++c) {
        for (int k = 0; k < 2; ++k) {
            lp_low_[c][k].setLowpass(fs, p.xover_low_hz, 0.707f);
            hp_low_[c][k].setHighpass(fs, p.xover_low_hz, 0.707f);
            lp_high_[c][k].setLowpass(fs, p.xover_high_hz, 0.707f);
            hp_high_[c][k].setHighpass(fs, p.xover_high_hz, 0.707f);
        }
    }
    for (int b = 0; b < 3; ++b) {
        band_[b].thr_db = p.comp_thr_db[b];
        band_[b].ratio  = std::max(1.0f, p.comp_ratio[b]);
        band_[b].atk    = msToCoeff(p.comp_atk_ms[b], fs);
        band_[b].rel    = msToCoeff(p.comp_rel_ms[b], fs);
        band_[b].makeup = std::pow(10.0f, p.comp_makeup_db[b] / 20.0f);
        band_[b].env    = 0.0f;
    }
}

void MultibandComp::reset() {
    for (int c = 0; c < 2; ++c)
        for (int k = 0; k < 2; ++k) {
            lp_low_[c][k].reset(); hp_low_[c][k].reset();
            lp_high_[c][k].reset(); hp_high_[c][k].reset();
        }
    for (int b = 0; b < 3; ++b) band_[b].env = 0.0f;
}

// Ganho do compressor (linear) para um nivel detectado (linear), banda b.
static inline float compGain(MultibandComp::Band& bd, float detect) {
    // Suavizacao de envelope com attack/release.
    float coeff = (detect > bd.env) ? bd.atk : bd.rel;
    bd.env = coeff * bd.env + (1.0f - coeff) * detect;
    float e = bd.env;
    if (e < 1e-7f) return bd.makeup;
    float level_db = 20.0f * std::log10(e);
    float gain_db = 0.0f;
    if (level_db > bd.thr_db) {
        float over = level_db - bd.thr_db;
        gain_db = -over * (1.0f - 1.0f / bd.ratio);
    }
    return std::pow(10.0f, gain_db / 20.0f) * bd.makeup;
}

void MultibandComp::process(float* L, float* R, int n, int ch) {
    for (int i = 0; i < n; ++i) {
        // ---- split canal L ----
        float xl = L[i];
        float lowL = lp_low_[0][1].process(lp_low_[0][0].process(xl));
        float upL  = hp_low_[0][1].process(hp_low_[0][0].process(xl));
        float midL = lp_high_[0][1].process(lp_high_[0][0].process(upL));
        float highL = hp_high_[0][1].process(hp_high_[0][0].process(upL));

        float lowR = 0, midR = 0, highR = 0;
        if (ch == 2) {
            float xr = R[i];
            lowR = lp_low_[1][1].process(lp_low_[1][0].process(xr));
            float upR = hp_low_[1][1].process(hp_low_[1][0].process(xr));
            midR = lp_high_[1][1].process(lp_high_[1][0].process(upR));
            highR = hp_high_[1][1].process(hp_high_[1][0].process(upR));
        }

        // ---- compressao estereo-linkada por banda ----
        float gLow  = compGain(band_[0], std::max(std::fabs(lowL),  std::fabs(lowR)));
        float gMid  = compGain(band_[1], std::max(std::fabs(midL),  std::fabs(midR)));
        float gHigh = compGain(band_[2], std::max(std::fabs(highL), std::fabs(highR)));

        L[i] = lowL * gLow + midL * gMid + highL * gHigh;
        if (ch == 2) R[i] = lowR * gLow + midR * gMid + highR * gHigh;
    }
}

} // namespace af
