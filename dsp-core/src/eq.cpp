#include "stages.h"

namespace af {

void EqStage::configure(float fs, const AfParams& p) {
    hpf_on_ = p.hpf_hz > 1.0f;
    for (int c = 0; c < 2; ++c) {
        if (hpf_on_) hpf_[c].setHighpass(fs, p.hpf_hz, 0.707f);
        pk1_[c].setPeaking(fs, p.peak1_hz, p.peak1_q, p.peak1_db);
        pk2_[c].setPeaking(fs, p.peak2_hz, p.peak2_q, p.peak2_db);
    }
}

void EqStage::reset() {
    for (int c = 0; c < 2; ++c) { hpf_[c].reset(); pk1_[c].reset(); pk2_[c].reset(); }
}

void EqStage::process(float* L, float* R, int n, int ch) {
    for (int i = 0; i < n; ++i) {
        float l = L[i];
        if (hpf_on_) l = hpf_[0].process(l);
        l = pk2_[0].process(pk1_[0].process(l));
        L[i] = l;
        if (ch == 2) {
            float r = R[i];
            if (hpf_on_) r = hpf_[1].process(r);
            r = pk2_[1].process(pk1_[1].process(r));
            R[i] = r;
        }
    }
}

} // namespace af
