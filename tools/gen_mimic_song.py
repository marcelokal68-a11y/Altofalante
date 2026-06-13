#!/usr/bin/env python3
"""Sintetiza um WAV que IMITA uma musica real (bateria + baixo + acordes + melodia),
com espectro cheio e dinamica — para calibrar/validar os presets e o auto-preset.

Nao usa nenhuma gravacao protegida; e 100% sintetizado. Saida: test-audio/mimic.wav
"""
import os
import numpy as np
import soundfile as sf

SR = 48000
BPM = 100
BEAT = 60 / BPM           # 0.6 s
BAR = 4 * BEAT            # 2.4 s
DUR = 12.0
N = int(SR * DUR)
T = np.arange(N) / SR
rng = np.random.default_rng(7)


def midi(m):
    return 440.0 * 2 ** ((m - 69) / 12)


def place(buf, start, sig):
    i0 = int(start * SR)
    i1 = min(N, i0 + len(sig))
    if i0 < N:
        buf[i0:i1] += sig[: i1 - i0]


def tone(freq, dur, wave="saw", harms=8):
    n = int(dur * SR)
    tt = np.arange(n) / SR
    env = np.exp(-tt * (2.5 / dur))
    s = np.zeros(n)
    if wave == "saw":
        for h in range(1, harms + 1):
            s += (1 / h) * np.sin(2 * np.pi * freq * h * tt)
    elif wave == "tri":
        for k in range(harms):
            h = 2 * k + 1
            s += ((-1) ** k) * (1 / h ** 2) * np.sin(2 * np.pi * freq * h * tt)
    else:
        s = np.sin(2 * np.pi * freq * tt)
    return env * s


def kick(dur=0.2):
    n = int(dur * SR)
    tt = np.arange(n) / SR
    f = 45 + 90 * np.exp(-tt * 30)
    ph = 2 * np.pi * np.cumsum(f) / SR
    return np.exp(-tt * 11) * np.sin(ph)


def bandlimit(x):
    # Limita a banda do ruido (~aprox. < 14 kHz), como audio real anti-aliasado.
    k = np.ones(5) / 5
    return np.convolve(x, k, mode="same")


def snare(dur=0.2):
    n = int(dur * SR)
    tt = np.arange(n) / SR
    noise = bandlimit(rng.standard_normal(n)) * np.exp(-tt * 18)
    tone_ = 0.5 * np.sin(2 * np.pi * 190 * tt) * np.exp(-tt * 22)
    return 0.7 * noise + tone_


def hat(dur=0.05):
    n = int(dur * SR)
    tt = np.arange(n) / SR
    return bandlimit(rng.standard_normal(n)) * np.exp(-tt * 90) * 0.5


def main():
    drums = np.zeros(N)
    bass = np.zeros(N)
    pad = np.zeros(N)
    lead = np.zeros(N)

    prog = [  # (raiz do baixo, triade do acorde) em MIDI
        (45, [57, 60, 64]),  # Am
        (41, [53, 57, 60]),  # F
        (48, [52, 55, 60]),  # C
        (43, [55, 59, 62]),  # G
    ]
    scale = [69, 72, 74, 76, 72, 69, 67, 72]  # melodia (A4..)

    nbars = int(DUR / BAR)
    for b in range(nbars):
        t0 = b * BAR
        root, triad = prog[b % 4]
        # acorde sustentado (pad), suave
        for m in triad:
            place(pad, t0, 0.12 * tone(midi(m), BAR * 0.95, "saw", 6))
        # baixo em colcheias
        for e in range(8):
            place(bass, t0 + e * (BEAT / 2), 0.5 * tone(midi(root), BEAT * 0.5, "saw", 5))
        # bateria
        for be in range(4):
            place(drums, t0 + be * BEAT, 0.95 * kick())
            if be in (1, 3):
                place(drums, t0 + be * BEAT, 0.6 * snare())
        for h in range(8):
            place(drums, t0 + h * (BEAT / 2), hat())
        # melodia (2 notas por compasso)
        place(lead, t0 + 0.0, 0.22 * tone(midi(scale[(b * 2) % 8]), BEAT * 1.5, "tri"))
        place(lead, t0 + BEAT * 2, 0.22 * tone(midi(scale[(b * 2 + 1) % 8]), BEAT * 1.5, "tri"))

    mono = 0.9 * drums + 0.8 * bass + 0.6 * pad + 0.7 * lead
    # leve estereo: pad/lead deslocados
    left = mono + 0.15 * np.roll(pad, 120) + 0.10 * lead
    right = mono - 0.15 * np.roll(pad, 120) - 0.10 * lead

    st = np.stack([left, right], axis=1).astype(np.float32)
    st *= (10 ** (-3 / 20)) / (np.max(np.abs(st)) + 1e-9)  # pico ~ -3 dBFS

    out_dir = os.path.join(os.path.dirname(__file__), "..", "test-audio")
    os.makedirs(out_dir, exist_ok=True)
    path = os.path.join(out_dir, "mimic.wav")
    sf.write(path, st, SR, subtype="FLOAT")
    print("gerado:", os.path.relpath(path), f"({DUR}s, {BPM} BPM, Am-F-C-G)")


if __name__ == "__main__":
    main()
