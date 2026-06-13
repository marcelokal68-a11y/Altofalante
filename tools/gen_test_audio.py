#!/usr/bin/env python3
"""Gera uma amostra estereo de teste com dinamica (para o compressor ter trabalho).

Mistura grave + medio + agudo com pulsos ritmicos (kick), em nivel moderado,
para simular musica. Saida: test-audio/sample.wav (float32).
"""
import os
import numpy as np
import soundfile as sf

SR = 48000
DUR = 6.0  # segundos


def main():
    n = int(SR * DUR)
    t = np.arange(n) / SR

    # Pulso ritmico (kick) a ~2 Hz para criar transientes/dinamica.
    beat = (np.sin(2 * np.pi * 2.0 * t) > 0.9).astype(float)
    env = np.zeros(n)
    decay = np.exp(-np.linspace(0, 1, int(SR * 0.25)) * 8)
    for i in np.where(np.diff(beat) > 0)[0]:
        end = min(i + len(decay), n)
        env[i:end] += decay[: end - i]
    env = np.clip(env, 0, 1)

    bass = 0.6 * np.sin(2 * np.pi * 60 * t) * (0.3 + 0.7 * env)   # grave + kick
    mid = 0.3 * np.sin(2 * np.pi * 330 * t)
    high = 0.15 * np.sin(2 * np.pi * 2500 * t) * (0.5 + 0.5 * env)

    mono = bass + mid + high
    # Estereo: leve diferenca de fase no agudo para ter conteudo de "side".
    left = mono + 0.05 * np.sin(2 * np.pi * 2500 * t + 0.5)
    right = mono - 0.05 * np.sin(2 * np.pi * 2500 * t + 0.5)

    stereo = np.stack([left, right], axis=1).astype(np.float32)
    # Normaliza para pico ~ -6 dBFS (deixa headroom; nivel moderado, "musica real").
    peak = np.max(np.abs(stereo))
    stereo *= (10 ** (-6 / 20)) / peak

    out_dir = os.path.join(os.path.dirname(__file__), "..", "test-audio")
    os.makedirs(out_dir, exist_ok=True)
    path = os.path.join(out_dir, "sample.wav")
    sf.write(path, stereo, SR, subtype="FLOAT")
    print(f"gerado: {os.path.relpath(path)} ({DUR}s, {SR} Hz, estereo)")


if __name__ == "__main__":
    main()
