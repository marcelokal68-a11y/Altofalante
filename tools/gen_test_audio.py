#!/usr/bin/env python3
"""Gera amostras estereo de teste de generos diferentes (para calibrar e validar o
auto-preset): musica equilibrada, voz/podcast e grave pesado.

Saidas em test-audio/: sample.wav (musica), sample_voice.wav, sample_bass.wav.
"""
import os
import numpy as np
import soundfile as sf

SR = 48000
DUR = 6.0


def normalize(x, peak_dbfs=-6.0):
    p = np.max(np.abs(x))
    return x * (10 ** (peak_dbfs / 20)) / (p + 1e-9)


def kick_env(t):
    beat = (np.sin(2 * np.pi * 2.0 * t) > 0.9).astype(float)
    env = np.zeros(len(t))
    decay = np.exp(-np.linspace(0, 1, int(SR * 0.25)) * 8)
    for i in np.where(np.diff(beat) > 0)[0]:
        end = min(i + len(decay), len(t))
        env[i:end] += decay[: end - i]
    return np.clip(env, 0, 1)


def music(t):
    # Musica: grave dinamico + medios + agudos (hi-hats), com transientes (crista alta).
    env = kick_env(t)
    bass = 0.6 * np.sin(2 * np.pi * 60 * t) * (0.15 + 0.85 * env)
    mid = 0.3 * np.sin(2 * np.pi * 330 * t) + 0.2 * np.sin(2 * np.pi * 660 * t)
    # hi-hats: rajadas de ruido agudo a cada 0.25 s
    hat = np.zeros(len(t))
    step = int(SR * 0.25)
    for s in range(0, len(t), step):
        e = min(s + int(SR * 0.04), len(t))
        hat[s:e] += np.random.randn(e - s) * np.exp(-np.linspace(0, 1, e - s) * 18)
    hat = np.convolve(hat, np.ones(5) / 5, mode="same")  # limita banda (audio real)
    high = 0.5 * hat + 0.12 * np.sin(2 * np.pi * 6000 * t)
    mono = bass + mid + high
    side = 0.06 * np.sin(2 * np.pi * 2500 * t + 0.5)
    return mono + side, mono - side


def voice(t):
    # Fala: energia nos medios (harmonicos >200 Hz), sem grave/agudo, baixa dinamica.
    f0 = 220 + 15 * np.sin(2 * np.pi * 0.7 * t)  # entonacao (acima de 200 Hz)
    trem = 0.7 + 0.3 * np.sin(2 * np.pi * 4 * t)  # silabas suaves
    v = (np.sin(2 * np.pi * f0 * t)
         + 0.9 * np.sin(2 * np.pi * 2 * f0 * t)
         + 0.7 * np.sin(2 * np.pi * 3 * f0 * t)
         + 0.4 * np.sin(2 * np.pi * 4 * f0 * t)) * trem
    v = np.tanh(2.5 * v)  # compressao tipo voz -> crista baixa
    return 0.5 * v, 0.5 * v


def bass(t):
    env = kick_env(t)
    sub = 0.8 * np.sin(2 * np.pi * 50 * t) * (0.2 + 0.8 * env)
    low = 0.5 * np.sin(2 * np.pi * 80 * t) * (0.3 + 0.7 * env)
    mono = sub + low + 0.06 * np.sin(2 * np.pi * 400 * t)
    return mono, mono


def main():
    t = np.arange(int(SR * DUR)) / SR
    out_dir = os.path.join(os.path.dirname(__file__), "..", "test-audio")
    os.makedirs(out_dir, exist_ok=True)
    for name, fn in [("sample", music), ("sample_voice", voice), ("sample_bass", bass)]:
        l, r = fn(t)
        st = normalize(np.stack([l, r], axis=1).astype(np.float32))
        path = os.path.join(out_dir, name + ".wav")
        sf.write(path, st, SR, subtype="FLOAT")
        print("gerado:", os.path.relpath(path))


if __name__ == "__main__":
    main()
