# Guia do Altofalante — usar, mexer e ensinar

Este guia é para **você** (mesmo sem ser programador) entender o app, mexer no básico e
ensinar outras pessoas.

---

## 1. O que o app faz (explique em 1 frase)
> "Ele turbina o som do celular — fica mais alto e mais limpo — e dá pra juntar vários
> celulares pra virar uma caixa de som maior."

Importante ser honesto: **não vira uma JBL**. Ele extrai o máximo que o auto-falante do
celular consegue, sem distorcer. O volume *de verdade* aumenta mesmo é juntando aparelhos.

---

## 2. Como rodar o app no seu computador

Você precisa instalar o **Flutter** uma vez: https://docs.flutter.dev/get-started/install

Depois, no terminal:
```bash
cd app
flutter create .      # cria as pastas do Android/iOS (só na 1ª vez)
flutter pub get       # baixa as dependências
flutter run           # roda num emulador ou celular conectado por USB
```
> ⚠️ O processamento de áudio em tempo real ainda precisa ser "ligado" no lado nativo
> (passo técnico descrito em `app/README.md`). Por enquanto a tela já funciona — os
> botões chamam o motor; falta plugar o motor de DSP no áudio do sistema.

---

## 3. Como usar (passo a passo)

1. **Escolher música:** toque no cartão de cima ("Toque para escolher uma música") e
   selecione um arquivo do celular.
2. **TURBO:** o botão grande no meio. **Ligado (rosa)** = som turbinado;
   **desligado (cinza)** = som original. Toque pra alternar e ouvir a diferença (teste A/B).
3. **Presets:** escolha o "tempero" do som:
   - **Voz** — podcast, fala, áudio de vídeo.
   - **Equilibrado** — música no geral (recomendado).
   - **Grave+** — mais corpo/grave.
   - **Festa** — volume no talo.
   - **Caixa portátil** — imita o timbre de uma caixinha Bluetooth.
4. **+ tocar junto com outro celular:** (em breve) junta outros aparelhos na mesma rede
   Wi-Fi pra tocarem em sincronia.
5. **Play/Pause** embaixo.

---

## 4. Como ensinar outra pessoa (roteiro de 30 segundos)
1. "Abre o app e escolhe uma música."
2. "Esse botão grande é o **Turbo**. Liga e desliga pra ouvir como muda."
3. "Esses botõezinhos são os **modos** — usa o *Equilibrado* pra música normal,
   *Festa* quando quiser no máximo."
4. "Dá play. Pronto, seu celular virou caixa de som."

Dica de demonstração: ponha a mesma música, alterne o Turbo a cada 5 segundos. A pessoa
**ouve** a diferença na hora — é o que mais convence.

---

## 5. Como mexer no app (onde está cada coisa)

| Quero mudar... | Arquivo | O que editar |
|----------------|---------|--------------|
| **Cores / visual** | `app/lib/theme.dart` | os valores em `Brand` (ex.: `primary2`, `gradient`) |
| **A tela / textos** | `app/lib/main.dart` | textos, tamanhos, posição dos botões |
| **O nome dos presets** | `app/lib/audio_engine.dart` | a lista `enum DspPreset` (rótulo e dica) |
| **Como o som é processado** | `dsp-core/src/presets.cpp` | os números de cada preset (ganho, graves, etc.) |
| **Logo / ícone** | `branding/` | os arquivos `.svg` (edite e gere `.png`) |

### Exemplo: deixar o app azul em vez de rosa
Em `app/lib/theme.dart`, troque as cores do `gradient` e de `primary2` por tons de azul.
Salve e rode `flutter run` de novo — a interface inteira acompanha.

### Exemplo: deixar o preset "Festa" ainda mais alto
Em `dsp-core/src/presets.cpp`, no bloco `AF_PRESET_PARTY`, aumente um pouco
`o->input_gain_db`. Depois rode `bash tools/demo.sh` para **medir** se ficou mais alto
sem estourar (o relatório mostra LUFS e true-peak). Esse é o jeito certo de calibrar:
mudar o número → medir → ouvir.

---

## 6. Material de marca
Tudo em `branding/`: `logo.png`, `app-icon.png`, `mockup-home.png` e o
`brand-guide.md` (cores, fontes, tom de voz). Use para posts, loja de apps e divulgação.
