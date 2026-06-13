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
sync-core/    # sincronização multi-celular em C++ + simulação (Fase 2)
tools/        # harness de teste/medição (roda no PC/CI, sem device)
app/          # esqueleto do app Flutter (Fase 3 — UI + ponte nativa)
branding/     # logo, ícone, mockup e guia de marca
docs/         # GUIA.md — como usar, mexer e ensinar
test-audio/   # amostras geradas (gitignored)
.github/      # CI (build + testes + prova de valor a cada push)
```

## Design & uso
- Identidade visual e mockups: [`branding/`](branding/) (ver `brand-guide.md`).
- Como usar o app, mexer no código e ensinar outros: [`docs/GUIA.md`](docs/GUIA.md).
- Testar a sincronização multi-celular (passo a passo): [`docs/TESTE-MULTICELULAR.md`](docs/TESTE-MULTICELULAR.md).

## Roadmap

- **Fase 1 — DSP + prova de valor** ✅ — loudness +5 a +6.8 LU sem clipping.
- **Fase 2 — Multi-celular sincronizado** ✅ protótipo completo — **descoberta
  automática** (multicast estilo mDNS, sem IP fixo) + sync de relógio + compensação de
  latência sobre **rede UDP real**: 3 nós sincronizam a **~0.01 ms**
  (`bash tools/sync_net_test.sh`), mesmo com relógios dessincronizados em dezenas de
  segundos. Falta só apontar para a interface Wi-Fi no device.
- **Fase 3 — App Flutter** 🚧 UI + branding prontos e **ponte nativa escrita**
  ([`app/native/`](app/native/) — AVAudioEngine no iOS, Oboe+JNI no Android, hospedando
  o `dsp-core`). Falta integrar no Xcode/Android Studio e verificar no device
  (ver [`app/NATIVE.md`](app/NATIVE.md)).

Ver [`specs/05-roadmap.md`](specs/05-roadmap.md).
