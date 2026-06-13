# Do zero ao celular — guia único

Dois caminhos. Comece pelo A (rápido, sem custo); o B é o app completo com multi-celular.

---

## 🟢 Caminho A — PWA (na vida real em ~30 min, sem Mac, sem contas, sem custo)

A PWA está pronta em `pwa/`. Ela roda no navegador do celular, instala na tela de
início e processa o áudio (player turbinado). Não tem o multi-celular.

1. **Hospede a pasta `pwa/`** com HTTPS (qualquer um destes, de graça):
   - **GitHub Pages:** Settings → Pages → publique a partir da branch apontando para
     `/pwa` (ou copie `pwa/` para uma branch `gh-pages`).
   - **Netlify / Vercel / Cloudflare Pages:** arraste a pasta `pwa/`.
2. Abra a URL **https://…** no celular.
3. **iPhone (Safari):** botão Compartilhar → "Adicionar à Tela de Início".
   **Android (Chrome):** aparece "Instalar app" / menu → "Adicionar à tela inicial".
4. Pronto — abra pelo ícone, escolha músicas e use o Turbo. 🎉

> Teste local antes: `cd pwa && python3 -m http.server 8000` → `http://localhost:8000`.

---

## 🔵 Caminho B — App nativo (iOS + Android, com multi-celular)

### 1. Ferramentas (na sua máquina)
- **iOS:** um **Mac** + **Xcode**. **Android:** **Android Studio** (+ NDK).
- **Flutter SDK:** https://docs.flutter.dev/get-started/install

### 2. Gerar o projeto e integrar a ponte nativa
```bash
cd app
flutter create .      # gera ios/ e android/
flutter pub get
```
Depois siga o **[`app/NATIVE.md`](../app/NATIVE.md)** para copiar `app/native/*` e
configurar Xcode (bridging + fontes C++) e Gradle (Oboe + NDK + CMake).

### 3. Permissões (cole estas — senão o multi-celular e o áudio em background falham)

**iOS — `ios/Runner/Info.plist`:**
```xml
<key>UIBackgroundModes</key>
<array><string>audio</string></array>
<key>NSLocalNetworkUsageDescription</key>
<string>Para tocar em sincronia com outros celulares na mesma Wi-Fi.</string>
<key>NSBonjourServices</key>
<array><string>_altofalante._udp</string></array>
```

**Android — `android/app/src/main/AndroidManifest.xml`** (dentro de `<manifest>`):
```xml
<uses-permission android:name="android.permission.INTERNET"/>
<uses-permission android:name="android.permission.ACCESS_WIFI_STATE"/>
<uses-permission android:name="android.permission.CHANGE_WIFI_MULTICAST_STATE"/>
<uses-permission android:name="android.permission.FOREGROUND_SERVICE"/>
<uses-permission android:name="android.permission.READ_MEDIA_AUDIO"/>
<uses-permission android:name="android.permission.MODIFY_AUDIO_SETTINGS"/>
```
(`MODIFY_AUDIO_SETTINGS` é para o "amplificar tudo" — efeitos de áudio globais.)
(No Android, ainda adquira um `MulticastLock` ao usar o multi-celular.)

### 4. Rodar no celular
```bash
flutter run      # com o aparelho conectado por USB
```
Teste o A/B do Turbo. Para o multi-celular entre 2 aparelhos, troque a interface
multicast de loopback para a Wi-Fi (env `AF_IFACE` ou `MCAST_IFACE` em
`sync-core/src/protocol.h`).

### 5. Preparar para as lojas
- Ícones/splash: `dart run flutter_launcher_icons` e `dart run flutter_native_splash:create`.
- Screenshots reais; **hospede `docs/privacidade.html`** e pegue a URL.
- Textos: `docs/LOJAS.md`. Build de release assinado (certificado iOS / keystore Android).

### 5b. Testar o "amplificar TUDO" num Android (gerar o APK)
Esse recurso é Android e turbina o som de qualquer app (Spotify/YouTube). Para testar:
```bash
cd app
flutter build apk --release
# saída: build/app/outputs/flutter-apk/app-release.apk
```
Instale no aparelho:
```bash
adb install build/app/outputs/flutter-apk/app-release.apk
# ou copie o .apk para o celular e abra (permita "instalar de fontes desconhecidas")
```
Depois: abra o app → ligue o **Turbo global** → toque algo no **Spotify/YouTube** e
ouça mais alto. Se o aparelho bloquear a sessão global, o app cai para o player próprio.

### 6. Contas e publicação
- **Apple Developer** (US$99/ano) e **Google Play** (US$25 única vez).
- Suba os builds, preencha as listagens, envie para revisão.

---

### Resumo
- **Quer validar já, sem custo?** → Caminho A (PWA).
- **Quer o produto completo (multi-celular, vitrine nas lojas)?** → Caminho B.
- Dá para fazer os dois: PWA agora para usuários reais, nativo em paralelo.
