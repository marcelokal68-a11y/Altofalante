# Ponte nativa — integração passo a passo

Liga o motor de DSP (`dsp-core`, C++) ao áudio real do celular. O código pronto está em
`app/native/`. Ele **não compila no ambiente remoto** — você faz isto na sua máquina,
com Flutter + Xcode (iOS) e/ou Android Studio (Android).

Pré-requisito: dentro de `app/`, rode uma vez `flutter create .` para gerar `ios/` e
`android/`. Depois siga abaixo.

---

## iOS (AVAudioEngine)

1. **Copie os arquivos** de `app/native/ios/` para `app/ios/Runner/`:
   - `AppDelegate.swift` (substitui o gerado)
   - `AudioEngineBridge.swift`
   - `Runner-Bridging-Header.h`
2. **Adicione o dsp-core ao projeto** (no Xcode, `Runner.xcworkspace`):
   - Arraste a pasta `dsp-core/src` para o target **Runner** (marque "Copy if needed"
     desmarcado; "Create groups"). Os `.cpp` entram na compilação.
3. **Build Settings do target Runner:**
   - **Objective-C Bridging Header:** `Runner/Runner-Bridging-Header.h`
   - **Header Search Paths:** adicione o caminho até `dsp-core/include`
     (ex.: `$(SRCROOT)/../../dsp-core/include`).
   - **C++ Language Dialect:** `GNU++17` (ou C++17).
4. **Rode:** `flutter run` com um iPhone conectado. Escolha uma música, ligue o **Turbo**.

> Permissões: tocar arquivos locais via `file_picker` não exige entitlement especial.

---

## Android (Oboe)

1. **Copie os arquivos:**
   - `app/native/android/MainActivity.kt` → `app/android/app/src/main/kotlin/<seu/pacote>/`
     (substitui o gerado; ajuste o `package` para o seu `applicationId`).
   - `app/native/android/native-lib.cpp` e `CMakeLists.txt`
     → `app/android/app/src/main/cpp/`.
2. **Dependência do Oboe + NDK + CMake** em `app/android/app/build.gradle`:
   ```gradle
   android {
       buildFeatures { prefab true }          // habilita o prefab (Oboe)
       externalNativeBuild {
           cmake { path "src/main/cpp/CMakeLists.txt" }
       }
       defaultConfig {
           ndk { abiFilters 'arm64-v8a', 'armeabi-v7a' }
       }
   }
   dependencies {
       implementation "com.google.oboe:oboe:1.8.0"
   }
   ```
3. **Confirme o caminho do dsp-core** no `CMakeLists.txt` (variável `DSP_DIR`) — o
   padrão assume que o `app/` está dentro do repositório.
4. **Ajuste os nomes JNI** em `native-lib.cpp` se o seu pacote não for
   `com.altofalante.app` (troque `Java_com_altofalante_app_MainActivity_...`).
5. **Rode:** `flutter run` com um Android conectado.

---

## Como funciona (resumo)
- O Flutter (`lib/audio_engine.dart`) envia comandos pelo MethodChannel
  `altofalante/engine`: `load`, `play`, `pause`, `setEnabled`, `setPreset`.
- iOS: `AVAudioSourceNode` entrega blocos; cada bloco passa por `af_process` (dsp-core).
- Android: callback do `Oboe` faz o mesmo, sobre o PCM decodificado pelo `MediaCodec`.
- Resultado: o som que sai do alto-falante já vem turbinado pelo mesmo motor que
  provamos no `tools/demo.sh` (+5 a +6.8 LU, sem clipping).

## Limitações do protótipo (próximos refinamentos)
- O áudio é decodificado inteiro para a memória (ok para músicas; depois, streaming).
- O sample-rate pedido ao Oboe/AVAudioEngine é o do arquivo; em casos raros pode
  precisar de reamostragem explícita.
- Integrar aqui o módulo de sync (`sync-core`) para o multi-celular dentro do app.
