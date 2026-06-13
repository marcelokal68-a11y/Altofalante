# 03 — Motor de DSP (pipeline)

O pipeline processa áudio float (intervalo [-1, 1]), interleaved, e é aplicado em blocos.
Ordem do sinal:

```
entrada → ganho → EQ paramétrico → compressor multibanda → exciter harmônico
        → alargamento estéreo → limitador true-peak → saída
```

## Por que essa ordem
- **Ganho** primeiro define o "drive" geral.
- **EQ** molda o timbre (corta sub-graves que o speaker não reproduz e que só roubam
  headroom; realça médios/presença onde o speaker do celular é eficiente).
- **Compressor multibanda** reduz a faixa dinâmica por banda → permite subir o nível
  médio (loudness) mantendo controle; é o que faz "soar mais alto".
- **Exciter** sintetiza harmônicos a partir dos graves; o cérebro "completa" o
  fundamental ausente (grave psicoacústico) — sensação de grave num speaker que não
  reproduz grave de verdade.
- **Alargamento estéreo** dá sensação de espaço (mono-compatível, sem destruir o centro).
- **Limitador true-peak** por último: teto absoluto. Garante saída sem clipping mesmo
  com todo o ganho anterior. **Sempre ativo.**

## Estágios e parâmetros

### 1. Gain
- `input_gain_db` (pré-ganho). Default 0 dB.

### 2. EQ paramétrico (cadeia de biquads)
Filtros configuráveis (tipo, freq, Q, ganho). Presets típicos:
- High-pass ~80–120 Hz (remove sub-graves inúteis no speaker, libera headroom).
- Realce de presença ~2–5 kHz (inteligibilidade/brilho).
- Leve corte em ~300–500 Hz se houver "abafamento".

### 3. Compressor multibanda
- 3 bandas (graves / médios / agudos) via crossover.
- Por banda: `threshold_db`, `ratio`, `attack_ms`, `release_ms`, `makeup_db`.
- Reduz dinâmica → headroom para subir loudness sem picos descontrolados.

### 4. Exciter harmônico
- `drive` (quantidade de harmônicos), `freq` de foco (banda dos graves a "completar").
- Gera harmônicos de ordem superior do conteúdo grave; mistura com o sinal seco
  (`mix`).

### 5. Alargamento estéreo (Mid/Side)
- `width` (1.0 = neutro; >1 alarga). Processa em M/S, mantém mono-compatibilidade.
- No-op se a entrada for mono.

### 6. Limitador true-peak
- `ceiling_dbtp` (default -1.0 dBTP), `release_ms`.
- Look-ahead curto + detecção de pico para impedir overshoot inter-amostra.
- **Invariante:** nenhuma amostra de saída ultrapassa o teto.

## Presets (v1)
| Preset                 | Caráter |
|------------------------|---------|
| Voz/Podcast            | HPF alto, presença em 3 kHz, compressão forte, sem exciter |
| Música — Equilibrado   | HPF moderado, leve presença, compressão média, exciter leve |
| Música — Grave+        | exciter forte, realce de graves controlado, width médio |
| Festa (loudness máx.)  | ganho + compressão agressivos, teto -1 dBTP, width amplo |
| Caixa portátil (voicing)| curva tipo "smiley" suave + exciter, simula timbre de speaker BT |

Os números exatos de cada preset vivem em `dsp-core/src/presets.cpp` e devem ser
ajustados com base nas medições do `measure.py` e em escuta no device.

## Invariantes de qualidade (testáveis)
- INV1: saída nunca excede `ceiling_dbtp` (teste do limitador com ganho extremo).
- INV2: com DSP ligado, LUFS de saída > LUFS de entrada (mais alto).
- INV3: processar com todos os ganhos em neutro ≈ passthrough (sanidade).
