/*
 * test_invariants — verifica as invariantes de qualidade do dsp-core
 * (ver specs/03-dsp-engine.md). Roda no container/CI, sem device.
 */
#include "altofalante/dsp.h"
#include <cstdio>
#include <cmath>
#include <vector>

static const float PI = 3.14159265358979f;

// Gera um seno estereo interleaved com amplitude `amp`.
static std::vector<float> sine(int sr, float freq, float amp, int frames) {
    std::vector<float> b(frames * 2);
    for (int i = 0; i < frames; ++i) {
        float s = amp * std::sin(2 * PI * freq * i / sr);
        b[2*i] = s; b[2*i+1] = s;
    }
    return b;
}

static float maxAbs(const std::vector<float>& b) {
    float m = 0; for (float v : b) m = std::fmax(m, std::fabs(v)); return m;
}
static float rms(const std::vector<float>& b) {
    double s = 0; for (float v : b) s += (double)v * v;
    return (float)std::sqrt(s / b.size());
}
static float dB(float lin) { return 20.0f * std::log10(lin + 1e-12f); }

static void processAll(AfEngine* e, std::vector<float>& b) {
    int frames = (int)b.size() / 2, block = 512;
    for (int i = 0; i < frames; i += block) {
        int n = (i + block <= frames) ? block : (frames - i);
        af_process(e, &b[(size_t)i * 2], n);
    }
}

int main() {
    const int sr = 48000, frames = sr; // 1 s
    int failures = 0;
    const float ceiling = std::pow(10.0f, -1.0f / 20.0f); // -1 dBFS

    // INV1: com sinal alto (0 dBFS) e o preset mais agressivo, a saida NUNCA
    // ultrapassa o teto do limitador.
    {
        auto buf = sine(sr, 220.0f, 1.0f, frames);
        AfEngine* e = af_create(sr, 2);
        af_set_preset(e, AF_PRESET_PARTY);
        processAll(e, buf);
        af_destroy(e);
        float peak = maxAbs(buf);
        bool ok = peak <= ceiling + 1e-4f;
        printf("[INV1] limitador: pico saida = %.4f (teto %.4f) -> %s\n",
               peak, ceiling, ok ? "OK" : "FALHOU");
        if (!ok) ++failures;
    }

    // INV2: com sinal de nivel moderado (-20 dBFS), o DSP aumenta o loudness
    // (RMS de saida > RMS de entrada).
    {
        auto in = sine(sr, 440.0f, 0.1f, frames); // ~ -20 dBFS
        auto out = in;
        AfEngine* e = af_create(sr, 2);
        af_set_preset(e, AF_PRESET_MUSIC_BALANCED);
        processAll(e, out);
        af_destroy(e);
        float ri = rms(in), ro = rms(out);
        bool ok = ro > ri;
        printf("[INV2] loudness: RMS in = %.1f dB, out = %.1f dB (ganho %.1f dB) -> %s\n",
               dB(ri), dB(ro), dB(ro) - dB(ri), ok ? "OK" : "FALHOU");
        if (!ok) ++failures;
    }

    // INV3: bypass = passthrough exato.
    {
        auto in = sine(sr, 1000.0f, 0.3f, frames);
        auto out = in;
        AfEngine* e = af_create(sr, 2);
        af_set_preset(e, AF_PRESET_BYPASS);
        processAll(e, out);
        af_destroy(e);
        float maxDiff = 0;
        for (size_t i = 0; i < in.size(); ++i)
            maxDiff = std::fmax(maxDiff, std::fabs(in[i] - out[i]));
        bool ok = maxDiff < 1e-7f;
        printf("[INV3] bypass passthrough: maxDiff = %.2e -> %s\n",
               maxDiff, ok ? "OK" : "FALHOU");
        if (!ok) ++failures;
    }

    printf("\n%s\n", failures == 0 ? "TODOS OS TESTES PASSARAM" : "HOUVE FALHAS");
    return failures == 0 ? 0 : 1;
}
