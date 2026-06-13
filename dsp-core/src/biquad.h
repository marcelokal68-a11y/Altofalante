/* Biquad filter (RBJ cookbook). Header-only. */
#ifndef ALTOFALANTE_BIQUAD_H
#define ALTOFALANTE_BIQUAD_H

#include <cmath>

namespace af {

class Biquad {
public:
    void reset() { z1_ = z2_ = 0.0f; }

    // Transposed Direct Form II.
    inline float process(float x) {
        float y = b0_ * x + z1_;
        z1_ = b1_ * x - a1_ * y + z2_;
        z2_ = b2_ * x - a2_ * y;
        return y;
    }

    void setCoeffs(float b0, float b1, float b2, float a0, float a1, float a2) {
        b0_ = b0 / a0; b1_ = b1 / a0; b2_ = b2 / a0;
        a1_ = a1 / a0; a2_ = a2 / a0;
    }

    void setLowpass(float fs, float f, float q) {
        float w0 = 2.0f * 3.14159265358979f * f / fs;
        float c = std::cos(w0), s = std::sin(w0);
        float alpha = s / (2.0f * q);
        setCoeffs((1 - c) * 0.5f, 1 - c, (1 - c) * 0.5f, 1 + alpha, -2 * c, 1 - alpha);
    }

    void setHighpass(float fs, float f, float q) {
        float w0 = 2.0f * 3.14159265358979f * f / fs;
        float c = std::cos(w0), s = std::sin(w0);
        float alpha = s / (2.0f * q);
        setCoeffs((1 + c) * 0.5f, -(1 + c), (1 + c) * 0.5f, 1 + alpha, -2 * c, 1 - alpha);
    }

    void setBandpass(float fs, float f, float q) {
        float w0 = 2.0f * 3.14159265358979f * f / fs;
        float c = std::cos(w0), s = std::sin(w0);
        float alpha = s / (2.0f * q);
        setCoeffs(alpha, 0.0f, -alpha, 1 + alpha, -2 * c, 1 - alpha);
    }

    void setPeaking(float fs, float f, float q, float gain_db) {
        float A = std::pow(10.0f, gain_db / 40.0f);
        float w0 = 2.0f * 3.14159265358979f * f / fs;
        float c = std::cos(w0), s = std::sin(w0);
        float alpha = s / (2.0f * q);
        setCoeffs(1 + alpha * A, -2 * c, 1 - alpha * A,
                  1 + alpha / A, -2 * c, 1 - alpha / A);
    }

private:
    float b0_ = 1, b1_ = 0, b2_ = 0, a1_ = 0, a2_ = 0;
    float z1_ = 0, z2_ = 0;
};

} // namespace af

#endif
