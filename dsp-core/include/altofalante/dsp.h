/*
 * Altofalante DSP core — public C API (estável para FFI).
 *
 * O mesmo motor roda no harness de teste (PC/CI), no iOS (AVAudioEngine) e no
 * Android (Oboe). O processamento e in-place sobre buffers float interleaved.
 */
#ifndef ALTOFALANTE_DSP_H
#define ALTOFALANTE_DSP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Presets prontos (ver specs/03-dsp-engine.md). */
typedef enum {
    AF_PRESET_BYPASS = 0,         /* passthrough (DSP desligado) */
    AF_PRESET_VOICE = 1,          /* voz / podcast */
    AF_PRESET_MUSIC_BALANCED = 2, /* musica - equilibrado */
    AF_PRESET_MUSIC_BASS = 3,     /* musica - grave+ */
    AF_PRESET_PARTY = 4,          /* festa - loudness maximo */
    AF_PRESET_PORTABLE = 5,       /* voicing "caixa portatil" */
    AF_PRESET_COUNT
} AfPreset;

/* Parametros completos do pipeline (para ajuste fino / calibracao). */
typedef struct {
    /* Gain */
    float input_gain_db;

    /* EQ: high-pass + 2 peaking. */
    float hpf_hz;            /* high-pass cutoff (0 = off) */
    float peak1_hz, peak1_db, peak1_q;
    float peak2_hz, peak2_db, peak2_q;

    /* Crossover do compressor multibanda (Hz). */
    float xover_low_hz;      /* low/mid split */
    float xover_high_hz;     /* mid/high split */

    /* Compressor por banda: threshold(dB), ratio, attack(ms), release(ms), makeup(dB) */
    float comp_thr_db[3];
    float comp_ratio[3];
    float comp_atk_ms[3];
    float comp_rel_ms[3];
    float comp_makeup_db[3];

    /* Exciter (grave psicoacustico) */
    float exciter_freq_hz;   /* topo da banda de graves a "completar" */
    float exciter_drive;     /* 0..1+ quantidade de harmonicos */
    float exciter_mix;       /* 0..1 mistura do sinal excitado */

    /* Alargamento estereo (M/S) */
    float stereo_width;      /* 1.0 = neutro, >1 alarga */

    /* Limitador true-peak */
    float limiter_ceiling_db; /* teto (dBFS), ex.: -1.0 */
    float limiter_release_ms;
} AfParams;

typedef struct AfEngine AfEngine;

/* Ciclo de vida. */
AfEngine* af_create(int sample_rate, int channels);
void      af_destroy(AfEngine* eng);

/* Configuracao. Seguras para chamar entre blocos (tempo real). */
void af_set_params(AfEngine* eng, const AfParams* params);
void af_set_preset(AfEngine* eng, AfPreset preset);
void af_set_enabled(AfEngine* eng, int enabled); /* 1 = DSP on, 0 = bypass (A/B) */

/* Preenche `out` com os parametros de um preset (sem precisar de engine). */
void af_get_preset_params(AfPreset preset, int sample_rate, AfParams* out);

/*
 * Processa `frames` quadros (amostras por canal) in-place.
 * `buffer` tem frames*channels floats, interleaved, em [-1, 1].
 */
void af_process(AfEngine* eng, float* buffer, int frames);

/* ---- EQ inteligente: análise de conteúdo e sugestão de preset ---- */

typedef struct {
    float low;    /* fração de energia nos graves (<200 Hz), 0..1 */
    float mid;    /* fração de energia nos médios (200-3k Hz), 0..1 */
    float high;   /* fração de energia nos agudos (>3k Hz), 0..1 */
    float crest;  /* fator de crista (pico/RMS) em dB — mede dinâmica */
} AfAnalysis;

/* Analisa um buffer (offline, não-realtime). Calcula balanço espectral e dinâmica. */
void af_analyze(const float* interleaved, int frames, int channels,
                int sample_rate, AfAnalysis* out);

/* Sugere o melhor preset a partir da análise (heurística). */
AfPreset af_suggest_preset(const AfAnalysis* a);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ALTOFALANTE_DSP_H */
