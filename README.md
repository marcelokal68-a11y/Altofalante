# Altofalante 🔊

App que transforma o celular numa caixa de som — amplifica o áudio com **mais volume e
clareza** via processamento de áudio (DSP) e, quando há vários aparelhos, sincroniza-os
para formar um sistema de som maior. Sem precisar de gadget externo.

> **O que é e o que não é.** O alto-falante do celular tem limite físico — software não
> cria grave real nem vira uma JBL. O ganho vem de DSP: o som fica **perceptivelmente
> mais alto e mais limpo/cheio**, sem distorcer. O volume *físico real* só aumenta de
> verdade juntando vários celulares (Fase 2). Veja [`specs/00-vision.md`](specs/00-vision.md).

## Estado atual — Fase 1 concluída ✅

O coração do produto (o motor de DSP) está implementado e **provado de forma medível**,
sem precisar de iPhone/Android. Resultado na amostra de teste:

| Preset    | LUFS   | True-peak  | ΔLUFS (mais alto) |
|-----------|--------|------------|-------------------|
| entrada   | -15.3  | -6.0 dBTP  | —                 |
| balanced  | -10.0  | -1.5 dBTP  | **+5.3**          |
| portable  |  -8.5  | -1.4 dBTP  | **+6.8**          |

Loudness **+5 a +6.8 LU mais alto, sem clipping** (todos ≤ -1 dBTP).

## Arquitetura

Um único **motor de DSP em C++** (`dsp-core/`), portátil, rodando igual no PC (teste),
iOS (AVAudioEngine) e Android (Oboe). A UI será **Flutter** (Fase 3) e só controla
parâmetros. Detalhes em [`specs/02-architecture.md`](specs/02-architecture.md).

Pipeline do sinal:
`ganho → EQ → compressor multibanda → exciter harmônico → alargamento estéreo → limitador true-peak`

(ver [`specs/03-dsp-engine.md`](specs/03-dsp-engine.md))

## Como rodar a prova de valor (sem device)

Requisitos: `cmake`, `g++`/`clang++`, Python 3 com `numpy soundfile pyloudnorm scipy`.

```bash
pip3 install numpy soundfile pyloudnorm scipy
bash tools/demo.sh
```

O script compila, roda os testes de invariante, gera uma amostra, processa todos os
presets e mede LUFS + true-peak (antes/depois).

### Manualmente
```bash
cmake -S . -B build && cmake --build build -j
./build/tools/test_invariants                       # testes de qualidade
python3 tools/gen_test_audio.py                     # gera test-audio/sample.wav
./build/tools/process_wav test-audio/sample.wav out.wav party
python3 tools/measure.py test-audio/sample.wav out.wav
```

## Estrutura

```
specs/        # spec-driven dev (visão, produto, arquitetura, DSP, sync, roadmap)
dsp-core/     # motor de DSP em C++ (lib portátil) — o coração do produto
tools/        # harness de teste/medição (roda no PC/CI, sem device)
test-audio/   # amostras geradas (gitignored)
app/          # app Flutter (Fase 3 — ainda não criado)
```

## Roadmap

- **Fase 1 — DSP + prova de valor** ✅ (este commit)
- **Fase 2 — Multi-celular sincronizado** (descoberta na rede, sync de relógio)
- **Fase 3 — App Flutter** (player + UI minimalista + integração nativa do DSP)

Ver [`specs/05-roadmap.md`](specs/05-roadmap.md).
