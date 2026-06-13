/*
 * analyze_wav — EQ inteligente: analisa um WAV e sugere o melhor preset.
 * Uso: analyze_wav <arquivo.wav>
 */
#include "altofalante/dsp.h"
#include "wav.h"
#include <cstdio>

static const char* presetName(AfPreset p) {
    switch (p) {
        case AF_PRESET_VOICE: return "Voz";
        case AF_PRESET_MUSIC_BALANCED: return "Equilibrado";
        case AF_PRESET_MUSIC_BASS: return "Grave+";
        case AF_PRESET_PARTY: return "Festa";
        case AF_PRESET_PORTABLE: return "Caixa portatil";
        default: return "Original";
    }
}

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "uso: %s <arquivo.wav>\n", argv[0]); return 1; }
    wav::Audio a;
    if (!wav::read(argv[1], a)) { fprintf(stderr, "erro lendo %s\n", argv[1]); return 2; }

    AfAnalysis an;
    af_analyze(a.samples.data(), a.frames(), a.channels, a.sample_rate, &an);
    AfPreset p = af_suggest_preset(&an);

    printf("%s\n", argv[1]);
    printf("  graves=%.0f%%  medios=%.0f%%  agudos=%.0f%%  crista=%.1f dB\n",
           an.low * 100, an.mid * 100, an.high * 100, an.crest);
    printf("  preset sugerido: %s\n", presetName(p));
    return 0;
}
