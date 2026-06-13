#include "altofalante/sync.h"

namespace af::sync {

OffsetEstimate estimateOffset(const std::vector<Sample>& samples) {
    OffsetEstimate best{0.0, 1e9};
    for (const auto& s : samples) {
        // round-trip = tempo total - tempo de processamento no lider.
        double delay = (s.t3 - s.t0) - (s.t2 - s.t1);
        if (delay < best.delay) {
            best.delay = delay;
            // offset (lider - seguidor) = ((t1-t0) + (t2-t3)) / 2
            best.offset = ((s.t1 - s.t0) + (s.t2 - s.t3)) * 0.5;
        }
    }
    return best;
}

} // namespace af::sync
