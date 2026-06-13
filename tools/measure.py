#!/usr/bin/env python3
"""Mede loudness (LUFS integrado, ITU-R BS.1770) e true-peak (dBTP) de arquivos WAV.

Uso:
    python3 measure.py <ref.wav> [outro.wav ...]

O primeiro arquivo e a referencia; os demais sao comparados a ele (delta de LUFS).
Prova objetivamente que o DSP aumenta o loudness sem clipping (true-peak <= -1 dBTP).
"""
import sys
import numpy as np
import soundfile as sf
import pyloudnorm as pyln
from scipy.signal import resample_poly


def true_peak_dbtp(x, sr):
    """True-peak via oversampling 4x (aproxima picos inter-amostra)."""
    if x.ndim == 1:
        x = x[:, None]
    up = resample_poly(x, 4, 1, axis=0)
    peak = np.max(np.abs(up))
    return 20 * np.log10(peak + 1e-12)


def measure(path):
    x, sr = sf.read(path, always_2d=True)
    meter = pyln.Meter(sr)
    lufs = meter.integrated_loudness(x)
    tp = true_peak_dbtp(x, sr)
    sample_peak = 20 * np.log10(np.max(np.abs(x)) + 1e-12)
    return lufs, tp, sample_peak


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    files = sys.argv[1:]
    print(f"{'arquivo':<28} {'LUFS':>8} {'TruePeak':>10} {'SmplPeak':>10} {'dLUFS':>8}")
    print("-" * 70)

    ref_lufs = None
    all_safe = True
    for i, f in enumerate(files):
        lufs, tp, sp = measure(f)
        if i == 0:
            ref_lufs = lufs
            delta = ""
        else:
            delta = f"{lufs - ref_lufs:+.1f}"
        name = f.split("/")[-1]
        clip_flag = "" if tp <= -1.0 + 1e-3 else "  <-- ACIMA de -1 dBTP!"
        if tp > -1.0 + 1e-3 and i > 0:
            all_safe = False
        print(f"{name:<28} {lufs:>8.1f} {tp:>9.1f}  {sp:>9.1f}  {delta:>8}{clip_flag}")

    print("-" * 70)
    if len(files) > 1:
        print("Meta: dLUFS >= +6 (mais alto) e TruePeak <= -1 dBTP (sem clipping).")
        print("Status anti-clipping:", "OK" if all_safe else "ATENCAO")


if __name__ == "__main__":
    main()
