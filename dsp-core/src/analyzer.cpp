#include "altofalante/dsp.h"
#include "biquad.h"
#include <cmath>
#include <vector>

// Analisa o conteúdo: balanço de energia em 3 bandas + fator de crista (dinâmica).
// Usa crossover LR4 (2 biquads) como o compressor multibanda.
extern "C" void af_analyze(const float* in, int frames, int channels,
                           int sample_rate, AfAnalysis* out) {
    using af::Biquad;
    const float fs = (float)sample_rate;
    Biquad lpLow[2], hpLow[2], lpHigh[2], hpHigh[2];
    for (int k = 0; k < 2; ++k) {
        lpLow[k].setLowpass(fs, 200.0f, 0.707f);
        hpLow[k].setHighpass(fs, 200.0f, 0.707f);
        lpHigh[k].setLowpass(fs, 3000.0f, 0.707f);
        hpHigh[k].setHighpass(fs, 3000.0f, 0.707f);
    }

    double eLow = 0, eMid = 0, eHigh = 0, eTot = 0;
    float peak = 0;
    for (int i = 0; i < frames; ++i) {
        // downmix mono
        float x = 0;
        for (int c = 0; c < channels; ++c) x += in[(size_t)i * channels + c];
        x /= channels;

        float low = lpLow[1].process(lpLow[0].process(x));
        float up = hpLow[1].process(hpLow[0].process(x));
        float mid = lpHigh[1].process(lpHigh[0].process(up));
        float high = hpHigh[1].process(hpHigh[0].process(up));

        eLow += (double)low * low;
        eMid += (double)mid * mid;
        eHigh += (double)high * high;
        eTot += (double)x * x;
        float a = std::fabs(x);
        if (a > peak) peak = a;
    }

    double sum = eLow + eMid + eHigh;
    if (sum < 1e-12) sum = 1e-12;
    out->low = (float)(eLow / sum);
    out->mid = (float)(eMid / sum);
    out->high = (float)(eHigh / sum);

    double rms = std::sqrt(eTot / std::max(1, frames));
    out->crest = (rms > 1e-9) ? 20.0f * std::log10(peak / (float)rms) : 0.0f;
}

extern "C" AfPreset af_suggest_preset(const AfAnalysis* a) {
    // Muito grave: festa se for dinâmico, senão realce de grave.
    if (a->low > 0.5f) return (a->crest > 12.0f) ? AF_PRESET_PARTY : AF_PRESET_MUSIC_BASS;
    // Fala/podcast: médios dominam, sem grave/agudo e pouca dinâmica.
    if (a->mid > 0.55f && a->high < 0.20f && a->crest < 10.0f) return AF_PRESET_VOICE;
    // Bem brilhante: voicing de caixa portátil.
    if (a->high > 0.28f) return AF_PRESET_PORTABLE;
    // Caso geral.
    return AF_PRESET_MUSIC_BALANCED;
}
