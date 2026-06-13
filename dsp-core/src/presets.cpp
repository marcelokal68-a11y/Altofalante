#include "altofalante/dsp.h"

extern "C" void af_get_preset_params(AfPreset preset, int /*sample_rate*/, AfParams* o) {
    // Base neutra.
    *o = AfParams{};
    o->input_gain_db = 0.0f;
    o->hpf_hz = 0.0f;
    o->peak1_hz = 1000; o->peak1_db = 0; o->peak1_q = 1.0f;
    o->peak2_hz = 4000; o->peak2_db = 0; o->peak2_q = 1.0f;
    o->xover_low_hz = 200; o->xover_high_hz = 3000;
    for (int b = 0; b < 3; ++b) {
        o->comp_thr_db[b] = -20; o->comp_ratio[b] = 1.0f;
        o->comp_atk_ms[b] = 10; o->comp_rel_ms[b] = 120; o->comp_makeup_db[b] = 0;
    }
    o->exciter_freq_hz = 130; o->exciter_drive = 0; o->exciter_mix = 0;
    o->stereo_width = 1.0f;
    o->limiter_ceiling_db = -1.5f; o->limiter_release_ms = 120;

    switch (preset) {
    case AF_PRESET_VOICE:
        o->input_gain_db = 8; o->hpf_hz = 120;
        o->peak1_hz = 300;  o->peak1_db = -2; o->peak1_q = 1.0f;
        o->peak2_hz = 3000; o->peak2_db = 4;  o->peak2_q = 1.0f;
        o->xover_low_hz = 250; o->xover_high_hz = 3000;
        o->comp_thr_db[0]=-20; o->comp_thr_db[1]=-24; o->comp_thr_db[2]=-20;
        o->comp_ratio[0]=3; o->comp_ratio[1]=4; o->comp_ratio[2]=3;
        o->comp_atk_ms[0]=5; o->comp_atk_ms[1]=3; o->comp_atk_ms[2]=5;
        o->comp_rel_ms[0]=120; o->comp_rel_ms[1]=80; o->comp_rel_ms[2]=120;
        o->comp_makeup_db[0]=3; o->comp_makeup_db[1]=6; o->comp_makeup_db[2]=3;
        break;
    case AF_PRESET_MUSIC_BALANCED:
        o->input_gain_db = 8; o->hpf_hz = 80;
        o->peak1_hz = 300;  o->peak1_db = -1.5f; o->peak1_q = 1.0f;
        o->peak2_hz = 4000; o->peak2_db = 3;     o->peak2_q = 0.9f;
        o->xover_low_hz = 200; o->xover_high_hz = 3000;
        o->comp_thr_db[0]=-22; o->comp_thr_db[1]=-22; o->comp_thr_db[2]=-22;
        o->comp_ratio[0]=2.5f; o->comp_ratio[1]=2.5f; o->comp_ratio[2]=2.5f;
        o->comp_atk_ms[0]=10; o->comp_atk_ms[1]=8; o->comp_atk_ms[2]=5;
        o->comp_rel_ms[0]=150; o->comp_rel_ms[1]=120; o->comp_rel_ms[2]=100;
        o->comp_makeup_db[0]=4; o->comp_makeup_db[1]=4; o->comp_makeup_db[2]=4;
        o->exciter_freq_hz = 120; o->exciter_drive = 0.4f; o->exciter_mix = 0.25f;
        o->stereo_width = 1.2f;
        break;
    case AF_PRESET_MUSIC_BASS:
        o->input_gain_db = 9; o->hpf_hz = 70;
        o->peak1_hz = 120;  o->peak1_db = 3;   o->peak1_q = 0.8f;
        o->peak2_hz = 5000; o->peak2_db = 2.5f; o->peak2_q = 0.9f;
        o->xover_low_hz = 180; o->xover_high_hz = 3000;
        o->comp_thr_db[0]=-24; o->comp_thr_db[1]=-22; o->comp_thr_db[2]=-22;
        o->comp_ratio[0]=3; o->comp_ratio[1]=2.5f; o->comp_ratio[2]=2.5f;
        o->comp_atk_ms[0]=8; o->comp_atk_ms[1]=8; o->comp_atk_ms[2]=5;
        o->comp_rel_ms[0]=140; o->comp_rel_ms[1]=120; o->comp_rel_ms[2]=100;
        o->comp_makeup_db[0]=6; o->comp_makeup_db[1]=4; o->comp_makeup_db[2]=4;
        o->exciter_freq_hz = 140; o->exciter_drive = 0.7f; o->exciter_mix = 0.4f;
        o->stereo_width = 1.3f;
        break;
    case AF_PRESET_PARTY:
        o->input_gain_db = 12; o->hpf_hz = 90;
        o->peak1_hz = 250;  o->peak1_db = -2;   o->peak1_q = 1.0f;
        o->peak2_hz = 4000; o->peak2_db = 3.5f; o->peak2_q = 0.9f;
        o->xover_low_hz = 200; o->xover_high_hz = 3500;
        o->comp_thr_db[0]=-28; o->comp_thr_db[1]=-28; o->comp_thr_db[2]=-28;
        o->comp_ratio[0]=4; o->comp_ratio[1]=4; o->comp_ratio[2]=4;
        o->comp_atk_ms[0]=3; o->comp_atk_ms[1]=3; o->comp_atk_ms[2]=2;
        o->comp_rel_ms[0]=100; o->comp_rel_ms[1]=90; o->comp_rel_ms[2]=80;
        o->comp_makeup_db[0]=8; o->comp_makeup_db[1]=8; o->comp_makeup_db[2]=8;
        o->exciter_freq_hz = 130; o->exciter_drive = 0.6f; o->exciter_mix = 0.35f;
        o->stereo_width = 1.4f; o->limiter_release_ms = 80;
        break;
    case AF_PRESET_PORTABLE:
        o->input_gain_db = 9; o->hpf_hz = 100;
        o->peak1_hz = 150;  o->peak1_db = 3; o->peak1_q = 0.7f;
        o->peak2_hz = 6000; o->peak2_db = 4; o->peak2_q = 0.7f;
        o->xover_low_hz = 200; o->xover_high_hz = 3500;
        o->comp_thr_db[0]=-24; o->comp_thr_db[1]=-20; o->comp_thr_db[2]=-22;
        o->comp_ratio[0]=3; o->comp_ratio[1]=2; o->comp_ratio[2]=2.5f;
        o->comp_atk_ms[0]=8; o->comp_atk_ms[1]=8; o->comp_atk_ms[2]=5;
        o->comp_rel_ms[0]=140; o->comp_rel_ms[1]=120; o->comp_rel_ms[2]=100;
        o->comp_makeup_db[0]=5; o->comp_makeup_db[1]=3; o->comp_makeup_db[2]=4;
        o->exciter_freq_hz = 140; o->exciter_drive = 0.6f; o->exciter_mix = 0.35f;
        o->stereo_width = 1.35f; o->limiter_release_ms = 110;
        break;
    case AF_PRESET_BYPASS:
    default:
        break; // neutro
    }
}
