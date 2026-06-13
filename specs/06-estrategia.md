# 06 — Estratégia de produto (os 3 pilares)

Decisão: perseguir os três caminhos como **uma estratégia só**, cada um no lugar certo.

## A realidade por plataforma (honesta)

| Capacidade | iPhone (iOS) | Android |
|---|---|---|
| Turbinar **arquivos no nosso player** | ✅ | ✅ |
| Turbinar **o áudio de outros apps** (Spotify/YouTube) | ❌ proibido pela Apple | ✅ **possível** (efeitos globais na sessão 0)\* |
| Multi-celular (vários aparelhos juntos) | ✅ | ✅ |

\* Depende do aparelho/versão; alguns OEMs restringem efeitos globais. Onde funciona, é o
nosso grande trunfo.

## Pilar 1 — Posicionamento honesto (iOS): "caixa de som das suas mídias"
No iPhone, assumir o limite: o app turbina **o que toca dentro dele** — músicas baixadas,
**podcasts**, áudios de WhatsApp, gravações. Mensagem honesta, sem prometer Spotify.
Público: quem tem arquivos/podcasts e quer ouvir mais alto e limpo sem caixa externa.

## Pilar 2 — Android: "amplifique TUDO" (o diferencial técnico)
No Android, usar **efeitos de áudio globais** (`LoudnessEnhancer` + `DynamicsProcessing`/
`Equalizer` + `BassBoost` na sessão de saída 0) para turbinar **qualquer som do
sistema** — Spotify, YouTube, jogos. É o "volume booster" de verdade que o iOS não
permite. Aqui o app entrega a promessa original para a maioria dos usuários (que streamam).
- Código: `app/native/android/SystemAudioEffects.kt`. Spec: `specs/07-android-system-audio.md`.
- Permissão: `MODIFY_AUDIO_SETTINGS`. Caveat: alguns aparelhos bloqueiam sessão 0.

## Pilar 3 — Multi-celular: o "uau" (festas)
Transformar vários celulares numa caixa de som sincronizada (estéreo/mono reforçado) é o
**diferencial de marketing** e a única forma de ganhar **volume físico real**. Já provado
(`sync-core`, ~0,01 ms). Vira a manchete: _"junte os celulares da galera = festa"_.

## Prioridades sugeridas
1. **Android "amplifique tudo"** (Pilar 2) — maior alcance e o "wow" de utilidade diária.
2. **Multi-celular** (Pilar 3) — diferencial de marketing, funciona nas duas plataformas.
3. **iOS player honesto** (Pilar 1) — já pronto (PWA + nativo); ajustar a comunicação.

## Reflexo na comunicação
- **iOS:** "suas músicas e podcasts, mais altos e cheios + junte celulares para festas".
- **Android:** "deixe QUALQUER som do celular mais alto (Spotify, YouTube...) + multi-celular".
- Loja: descrições por plataforma (Android destaca "amplifica tudo"; iOS destaca player+podcast).
