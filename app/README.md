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

## Cabeamento do motor nativo

O código nativo que hospeda o `dsp-core` **já está escrito** em
[`native/`](native/): iOS (`AVAudioEngine` + `AVAudioSourceNode`) e Android
(`Oboe` + JNI), ambos chamando `af_process(...)` a cada bloco. O passo a passo de
integração (Xcode/Android Studio, Gradle, dependência do Oboe) está em
[`NATIVE.md`](NATIVE.md).

O contrato C do motor está em
[`../dsp-core/include/altofalante/dsp.h`](../dsp-core/include/altofalante/dsp.h).

## Mapa de telas
- `lib/main.dart` — tela única (Turbo, presets, play, escolher faixa).
- `lib/audio_engine.dart` — cliente do `MethodChannel` + presets.
