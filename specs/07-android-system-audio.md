# 07 — Android: amplificar o áudio do sistema (Pilar 2)

No Android (diferente do iOS) é possível anexar **efeitos de áudio à sessão de saída
global (session 0)**, que afeta **todo** o som do aparelho — incluindo Spotify, YouTube,
jogos. É o "volume booster" de verdade.

## APIs usadas (android.media.audiofx)
- **`LoudnessEnhancer(0)`** — `setTargetGain(mB)`: aumenta o loudness percebido de tudo.
  É o coração do "deixar mais alto" global. (mB = milibéis; 600 = +6 dB.)
- **`DynamicsProcessing(0)`** (API 28+) — compressor multibanda + limitador + EQ globais
  (espelha o nosso `dsp-core`, mas aplicado ao mix do sistema).
- **`Equalizer(prio, 0)`** e **`BassBoost(prio, 0)`** — realce de presença/grave.

> `session 0` = mix de saída global. Onde o aparelho permite, os efeitos pegam em tudo.

## Limites honestos
- **Não é universal:** alguns OEMs/versões restringem efeitos na sessão 0 (segurança).
  O app deve detectar falha e cair de volta para o **player próprio** (turbina o que toca
  nele) como garantia.
- **Sem ultrapassar o máximo físico** do alto-falante (vale a mesma física do iOS).
- **Limitador obrigatório** (DynamicsProcessing) para o boost não distorcer.

## Permissões
- `MODIFY_AUDIO_SETTINGS` (manifest).
- Em alguns aparelhos, o usuário precisa permitir o app como "controle de áudio".

## Plano
1. v1: `LoudnessEnhancer` + `BassBoost` + `Equalizer` globais com um botão Turbo e
   presets (mapear os mesmos nomes do `dsp-core`).
2. v2: trocar por `DynamicsProcessing` (compressor multibanda + limitador) para o mesmo
   caráter do nosso motor C++.
3. Fallback automático para o player próprio quando a sessão global não for permitida.

## Implementação
`app/native/android/SystemAudioEffects.kt` (Kotlin puro, sem C++) + canal
`altofalante/system` (enableBoost/setGain/setPreset/disable). Ver `app/NATIVE.md`.
