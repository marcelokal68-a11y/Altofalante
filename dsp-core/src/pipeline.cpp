#include "altofalante/dsp.h"
#include "stages.h"
#include <vector>
#include <algorithm>

struct AfEngine {
    int sample_rate = 48000;
    int channels = 2;
    bool enabled = true;
    AfPreset preset = AF_PRESET_BYPASS;

    af::GainStage     gain;
    af::EqStage       eq;
    af::MultibandComp comp;
    af::Exciter       exciter;
    af::StereoWiden   widen;
    af::Limiter       limiter;

    std::vector<float> L, R;

    void configure(const AfParams& p) {
        gain.setGainDb(p.input_gain_db);
        eq.configure((float)sample_rate, p);
        comp.configure((float)sample_rate, p);
        exciter.configure((float)sample_rate, p);
        widen.setWidth(p.stereo_width);
        limiter.configure((float)sample_rate, p);
    }
    void reset() {
        eq.reset(); comp.reset(); exciter.reset(); limiter.reset();
    }
};

extern "C" {

AfEngine* af_create(int sample_rate, int channels) {
    AfEngine* e = new AfEngine();
    e->sample_rate = sample_rate;
    e->channels = std::max(1, std::min(2, channels));
    AfParams p; af_get_preset_params(AF_PRESET_BYPASS, sample_rate, &p);
    e->configure(p);
    e->reset();
    return e;
}

void af_destroy(AfEngine* e) { delete e; }

void af_set_params(AfEngine* e, const AfParams* p) {
    if (e && p) e->configure(*p);
}

void af_set_preset(AfEngine* e, AfPreset preset) {
    if (!e) return;
    e->preset = preset;
    AfParams p; af_get_preset_params(preset, e->sample_rate, &p);
    e->configure(p);
}

void af_set_enabled(AfEngine* e, int enabled) {
    if (e) e->enabled = (enabled != 0);
}

void af_process(AfEngine* e, float* buffer, int frames) {
    if (!e || !buffer || frames <= 0) return;
    // Bypass = passthrough exato.
    if (!e->enabled || e->preset == AF_PRESET_BYPASS) return;

    const int ch = e->channels;
    e->L.resize(frames);
    if (ch == 2) e->R.resize(frames);
    float* L = e->L.data();
    float* R = (ch == 2) ? e->R.data() : nullptr;

    // Deinterleave.
    if (ch == 2) {
        for (int i = 0; i < frames; ++i) { L[i] = buffer[2*i]; R[i] = buffer[2*i+1]; }
    } else {
        for (int i = 0; i < frames; ++i) L[i] = buffer[i];
    }

    // Pipeline (ordem do sinal — ver specs/03-dsp-engine.md).
    e->gain.process(L, R, frames, ch);
    e->eq.process(L, R, frames, ch);
    e->comp.process(L, R, frames, ch);
    e->exciter.process(L, R, frames, ch);
    e->widen.process(L, R, frames, ch);
    e->limiter.process(L, R, frames, ch);

    // Interleave de volta.
    if (ch == 2) {
        for (int i = 0; i < frames; ++i) { buffer[2*i] = L[i]; buffer[2*i+1] = R[i]; }
    } else {
        for (int i = 0; i < frames; ++i) buffer[i] = L[i];
    }
}

} // extern "C"
