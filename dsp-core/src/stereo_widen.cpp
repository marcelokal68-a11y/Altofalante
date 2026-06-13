#include "stages.h"

namespace af {

// Processa em Mid/Side: mantem o centro (mono-compativel) e escala o lado.
void StereoWiden::process(float* L, float* R, int n, int ch) {
    if (ch != 2 || width_ == 1.0f) return;
    for (int i = 0; i < n; ++i) {
        float mid = 0.5f * (L[i] + R[i]);
        float side = 0.5f * (L[i] - R[i]) * width_;
        L[i] = mid + side;
        R[i] = mid - side;
    }
}

} // namespace af
