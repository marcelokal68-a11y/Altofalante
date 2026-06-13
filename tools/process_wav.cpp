/*
 * process_wav — harness de teste do dsp-core (sem device).
 * Le um WAV, aplica o pipeline em blocos (como um render callback), grava o WAV.
 *
 * Uso: process_wav <in.wav> <out.wav> [preset]
 *   preset: bypass|voice|balanced|bass|party|portable  (default: balanced)
 */
#include "altofalante/dsp.h"
#include "wav.h"
#include <cstdio>
#include <cstring>
#include <string>

static AfPreset parsePreset(const char* s) {
    if (!strcmp(s, "bypass"))   return AF_PRESET_BYPASS;
    if (!strcmp(s, "voice"))    return AF_PRESET_VOICE;
    if (!strcmp(s, "balanced")) return AF_PRESET_MUSIC_BALANCED;
    if (!strcmp(s, "bass"))     return AF_PRESET_MUSIC_BASS;
    if (!strcmp(s, "party"))    return AF_PRESET_PARTY;
    if (!strcmp(s, "portable")) return AF_PRESET_PORTABLE;
    return AF_PRESET_MUSIC_BALANCED;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "uso: %s <in.wav> <out.wav> [preset]\n", argv[0]);
        return 1;
    }
    AfPreset preset = (argc >= 4) ? parsePreset(argv[3]) : AF_PRESET_MUSIC_BALANCED;

    wav::Audio a;
    if (!wav::read(argv[1], a)) { fprintf(stderr, "erro lendo %s\n", argv[1]); return 2; }
    printf("entrada: %d Hz, %d canais, %d frames\n", a.sample_rate, a.channels, a.frames());

    AfEngine* eng = af_create(a.sample_rate, a.channels);
    af_set_preset(eng, preset);

    // Processa em blocos de 512 frames (simula buffer de audio em tempo real).
    const int block = 512;
    int total = a.frames();
    for (int i = 0; i < total; i += block) {
        int n = (i + block <= total) ? block : (total - i);
        af_process(eng, &a.samples[(size_t)i * a.channels], n);
    }
    af_destroy(eng);

    if (!wav::write(argv[2], a)) { fprintf(stderr, "erro gravando %s\n", argv[2]); return 3; }
    printf("saida: %s (preset=%s)\n", argv[2], argc >= 4 ? argv[3] : "balanced");
    return 0;
}
