# 02 — Arquitetura

## Princípio central
**Um único motor de DSP em C++**, compartilhado por iOS e Android, hospedado pelo motor
de áudio nativo de cada plataforma. A UI é multiplataforma (Flutter) e só controla
parâmetros — nunca processa amostras de áudio.

```
┌──────────────────────────────────────────────┐
│                 UI (Flutter)                   │  Dart
│   tela única, presets, botão Turbo, A/B        │
└───────────────┬────────────────────────────────┘
                │ parâmetros (FFI / MethodChannel)
┌───────────────▼────────────────────────────────┐
│         Motor de áudio nativo                   │
│  iOS:  AVAudioEngine + render callback          │  Swift / ObjC
│  Android: Oboe / AAudio (baixa latência)        │  Kotlin / C++
└───────────────┬────────────────────────────────┘
                │ buffers de áudio (float interleaved)
┌───────────────▼────────────────────────────────┐
│           dsp-core  (C++ portátil)              │  <- coração do produto
│  Pipeline: gain → EQ → multiband comp →         │
│            exciter → stereo widen → limiter     │
└─────────────────────────────────────────────────┘
```

## Por que essa divisão
- O DSP é a parte difícil e a que define o produto; escrevê-lo **uma vez** em C++ evita
  divergência entre plataformas e permite testá-lo **fora do dispositivo** (no PC/CI).
- A UI muda pouco e não tem requisito de tempo real → Flutter dá velocidade de
  desenvolvimento e visual consistente nos dois sistemas.

## Contrato do dsp-core (API estável p/ FFI)
Definido em `dsp-core/include/altofalante/dsp.h`. Em resumo:
- `af_create(sample_rate, channels) -> handle`
- `af_set_params(handle, AfParams*)` — atualiza presets/ganhos em tempo real
- `af_process(handle, float* interleaved, int frames)` — processa **in-place**
- `af_destroy(handle)`

O processamento é **in-place** e **stateless entre chamadas exceto pelo estado interno
dos filtros/compressor/limiter** (que persiste no handle). Isso casa com o modelo de
*render callback* das duas plataformas.

## Fluxo de áudio em tempo real (no device — fase 3)
1. Motor nativo decodifica a faixa → buffers PCM float.
2. Buffer entregue ao `af_process`.
3. Buffer processado vai para a saída (alto-falante).
4. UI atualiza parâmetros via `af_set_params` quando o usuário troca preset/Turbo.

## Harness de teste (fase 1 — sem device)
`tools/process_wav.cpp` substitui o motor nativo: lê um WAV, chama `af_process` em
blocos, escreve um WAV processado. `tools/measure.py` mede LUFS e true-peak de entrada
e saída para provar o ganho objetivamente. Isso roda no container/CI.

## Stack resumida
| Camada            | iOS              | Android          | Teste (fase 1) |
|-------------------|------------------|------------------|----------------|
| UI                | Flutter          | Flutter          | —              |
| Motor de áudio    | AVAudioEngine    | Oboe/AAudio      | process_wav    |
| DSP               | dsp-core (C++)   | dsp-core (C++)   | dsp-core (C++) |
