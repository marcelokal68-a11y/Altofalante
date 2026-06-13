/* Declaracoes dos estagios do pipeline. Todos operam em buffers L/R separados. */
#ifndef ALTOFALANTE_STAGES_H
#define ALTOFALANTE_STAGES_H

#include <vector>
#include "biquad.h"
#include "altofalante/dsp.h"

namespace af {

// 1. Ganho de entrada.
class GainStage {
public:
    void setGainDb(float db);
    void process(float* L, float* R, int n, int ch);
private:
    float gain_ = 1.0f;
};

// 2. EQ: high-pass + 2 filtros peaking, por canal.
class EqStage {
public:
    void configure(float fs, const AfParams& p);
    void reset();
    void process(float* L, float* R, int n, int ch);
private:
    bool  hpf_on_ = false;
    Biquad hpf_[2], pk1_[2], pk2_[2];
};

// 3. Compressor multibanda (3 bandas, estereo-linkado).
class MultibandComp {
public:
    struct Band { float thr_db, ratio, atk, rel, makeup; float env = 0.0f; };
    void configure(float fs, const AfParams& p);
    void reset();
    void process(float* L, float* R, int n, int ch);
private:
    // Crossover LR4 (2 biquads cada) por canal.
    Biquad lp_low_[2][2];     // low band lowpass
    Biquad hp_low_[2][2];     // separa o que esta acima do xover baixo
    Biquad lp_high_[2][2];    // mid band lowpass (do residuo)
    Biquad hp_high_[2][2];    // high band highpass (do residuo)
    Band   band_[3];
    float  fs_ = 48000.0f;
};

// 4. Exciter harmonico (grave psicoacustico).
class Exciter {
public:
    void configure(float fs, const AfParams& p);
    void reset();
    void process(float* L, float* R, int n, int ch);
private:
    float drive_ = 0.0f, mix_ = 0.0f;
    Biquad lp_[2];   // isola a banda de graves
    Biquad hp_[2];   // mantem so os harmonicos gerados
};

// 5. Alargamento estereo (M/S).
class StereoWiden {
public:
    void setWidth(float w) { width_ = w; }
    void process(float* L, float* R, int n, int ch);
private:
    float width_ = 1.0f;
};

// 6. Limitador com look-ahead (estereo-linkado) + clip de seguranca.
class Limiter {
public:
    void configure(float fs, const AfParams& p);
    void reset();
    void process(float* L, float* R, int n, int ch);
private:
    float ceiling_ = 0.891f; // ~ -1 dBFS
    float atk_ = 0.0f, rel_ = 0.0f;
    float gain_ = 1.0f;
    int   la_ = 0;            // look-ahead em amostras
    std::vector<float> dl_[2];
    int   widx_ = 0;
};

} // namespace af

#endif
