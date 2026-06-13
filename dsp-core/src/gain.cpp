#include "stages.h"
#include <cmath>

namespace af {

void GainStage::setGainDb(float db) {
    gain_ = std::pow(10.0f, db / 20.0f);
}

void GainStage::process(float* L, float* R, int n, int ch) {
    for (int i = 0; i < n; ++i) {
        L[i] *= gain_;
        if (ch == 2) R[i] *= gain_;
    }
}

} // namespace af
