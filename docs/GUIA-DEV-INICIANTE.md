# Do zero ao app rodando — guia para iniciante total (Android)

> Para quem **nunca** programou. Vamos devagar. Você vai ver mensagens estranhas e erros
> — **é normal**. Quando travar, copie o erro e peça ajuda. Reserve umas 2–3 horas.

## O que você precisa antes de começar
- Um **computador** (Windows, Mac ou Linux). _Não dá para fazer no celular._
- Um **celular Android** com cabo USB (ou usaremos um emulador no computador).
- Paciência e conexão de internet.

---

## Fase 1 — Instalar as ferramentas (uma vez só)

1. **Android Studio** (vem com quase tudo): baixe em
   https://developer.android.com/studio e instale (vai clicando "Next/Avançar").
   - Na primeira abertura, ele baixa o "Android SDK" sozinho — deixe baixar.
2. **Flutter** (o motor do app): siga
   https://docs.flutter.dev/get-started/install → escolha seu sistema → siga os passos.
3. **Conferir:** abra o **Terminal** (Prompt de Comando no Windows) e digite:
   ```
   flutter doctor
   ```
   Ele lista o que falta com ✓ e ✗. Resolva os ✗ que ele indicar (ou me mande a tela).

## Fase 2 — Baixar o projeto
1. Acesse https://github.com/marcelokal68-a11y/Altofalante
2. Botão verde **"Code" → "Download ZIP"**. Descompacte numa pasta fácil (ex.: Documentos).
3. No Terminal, entre na pasta `app` do projeto. Ex. (ajuste o caminho):
   ```
   cd Documentos/Altofalante/app
   ```

## Fase 3 — Preparar o app
```
flutter create .
flutter pub get
```
- `flutter create .` cria as pastas do Android/iOS.
- `flutter pub get` baixa as dependências.

## Fase 4 — Colocar a parte "nativa" (copiar arquivos)
Siga **[`app/NATIVE.md`](../app/NATIVE.md)** (seção Android):
- Copie `app/native/android/*` para os lugares indicados.
- Adicione as permissões no `AndroidManifest.xml` (lista em
  [`DO-ZERO-AO-CELULAR.md`](DO-ZERO-AO-CELULAR.md)).
- Confirme o caminho do `dsp-core`/`sync-core` no `CMakeLists.txt`.
> Esta é a parte mais técnica. Se travar, me mande o arquivo e o erro — fazemos juntos.

## Fase 5 — Rodar no celular
1. No Android: Ajustes → Sobre o telefone → toque 7x em "Número da versão" para virar
   desenvolvedor → ative a **Depuração USB**.
2. Conecte o celular por USB (aceite o aviso no telefone).
3. No Terminal, dentro de `app`:
   ```
   flutter run
   ```
4. O app abre no seu celular. 🎉 (Sem celular? No Android Studio crie um emulador.)

## Fase 6 — Gerar o APK para instalar/vender
```
flutter build apk --release
```
Arquivo em `build/app/outputs/flutter-apk/app-release.apk`.

---

## Quando der erro (vai dar, e tudo bem)
- **Copie a mensagem inteira** e me mande aqui. Eu explico em português simples e digo o
  que fazer.
- Erros comuns: faltou aceitar licença do SDK (`flutter doctor --android-licenses`),
  versão do Java, caminho errado no `CMakeLists.txt`. Todos têm solução.

## Atalho se empacar
Se a fase nativa (4) ficar difícil, dá para **publicar primeiro só o player** (sem o
"amplificar tudo"), que é mais simples, e adicionar o resto depois. Me avise que ajusto.
