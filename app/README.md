# Altofalante — app Flutter (Fase 3, esqueleto)

UI multiplataforma (iOS + Android) com **tela única**: botão Turbo, presets e play.
O processamento de áudio em tempo real roda no **lado nativo**, hospedando o
`dsp-core` (C++); o Dart só controla parâmetros via `MethodChannel`
(ver [`../specs/02-architecture.md`](../specs/02-architecture.md)).

> ⚠️ Este diretório é um **esqueleto**. As pastas nativas (`ios/`, `android/`) e o
> `.dart_tool/` são geradas pelo Flutter na sua máquina — **não rodam no ambiente
> remoto** (sem SDK do Flutter / sem device). Os passos abaixo são para rodar local.

## Como inicializar e rodar (na sua máquina)

```bash
cd app
flutter create .                # gera ios/, android/, etc. preservando lib/ e pubspec
flutter pub get
flutter run                     # com um device/emulador conectado
```

## Cabeamento do motor nativo (a fazer na Fase 3)

O `MethodChannel('altofalante/engine')` (ver `lib/audio_engine.dart`) precisa de uma
implementação nativa que hospede o `dsp-core`:

- **iOS (Swift):** `AVAudioEngine` com um *render callback* / `AVAudioSourceNode` que,
  a cada buffer, chama `af_process(...)` do `dsp-core`. Métodos do canal: `load`,
  `play`, `pause`, `setEnabled`, `setPreset`. Linkar o C++ via um *bridging* objc++/
  CocoaPods apontando para `../dsp-core`.
- **Android (Kotlin + C++):** `Oboe`/`AAudio` num `AudioStream` callback chamando
  `af_process(...)` via JNI. Linkar `../dsp-core` no `CMakeLists.txt` do módulo
  (`externalNativeBuild`).

O contrato C do motor está em
[`../dsp-core/include/altofalante/dsp.h`](../dsp-core/include/altofalante/dsp.h).

## Mapa de telas
- `lib/main.dart` — tela única (Turbo, presets, play, escolher faixa).
- `lib/audio_engine.dart` — cliente do `MethodChannel` + presets.
