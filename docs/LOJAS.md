# Publicação nas lojas — textos e assets

Tudo o que você precisa para listar o **Altofalante** na App Store e no Google Play.
Assets prontos em `branding/icons/` e `branding/store/`.

---

## Assets visuais
| Item | Arquivo | Uso |
|------|---------|-----|
| Ícone mestre 1024² | `branding/icons/icon-1024.png` | App Store + gerar os demais |
| Ícone Play 512² | `branding/icons/playstore-512.png` | Google Play |
| Mipmaps Android | `branding/icons/ic_launcher-*.png` | mdpi…xxxhdpi |
| Arte de destaque | `branding/store/feature-graphic.png` (1024×500) | Google Play "feature graphic" |
| Splash | `branding/store/splash-logo.png` | tela de abertura |

> **Screenshots:** use o preview (`preview/altofalante.html`) ou o app rodando no
> device. A Play exige 2–8 prints; a App Store, por tamanho de tela (6.7" e 6.5").

---

## Geração automática no Flutter (recomendado)
Já deixei a configuração em `app/pubspec.yaml`. Na sua máquina:
```bash
cd app
flutter pub get
dart run flutter_launcher_icons      # gera todos os ícones iOS/Android
dart run flutter_native_splash:create # gera a splash
```

---

## Textos — Google Play

**Título (até 30):**
`Altofalante: caixa de som`

**Descrição curta (até 80):**
`Turbine o som do seu celular e junte vários aparelhos numa caixa de som só.`

**Descrição completa:**
```
Seu celular toca baixo demais? O Altofalante extrai o máximo de volume e clareza
do alto-falante do seu aparelho — com um motor de áudio profissional que deixa o som
mais ALTO e mais LIMPO, sem distorcer.

E tem mais: junte vários celulares na mesma Wi-Fi e eles tocam JUNTOS, em sincronia,
como um sistema de som maior. Dá até para usar dois celulares em estéreo — um vira o
lado esquerdo, o outro o direito.

✔ Botão TURBO: um toque para turbinar o som
✔ Modos prontos: Voz, Equilibrado, Grave+, Festa e Caixa portátil
✔ Multi-celular sincronizado (estéreo de verdade)
✔ Simples: abrir e tocar, sem configurar nada
✔ Processa tudo no aparelho — sua música não vai para lugar nenhum

Honestidade: nenhum app transforma o celular numa caixa JBL (existe um limite físico).
O Altofalante entrega o máximo que o seu aparelho consegue — e, com vários celulares,
o volume aumenta de verdade.

Toque suas músicas e transforme qualquer lugar numa festa. 🔊
```

**Categoria:** Música e áudio
**Tags:** amplificador de som, volume booster, equalizador, caixa de som, alto-falante

> **Destaque Android (Pilar 2):** no Android, abra com _"Deixe QUALQUER som do celular
> mais alto — Spotify, YouTube, jogos — com um toque."_ É o diferencial que o iPhone não
> tem. No iOS, manter o texto focado em "suas músicas e podcasts" + multi-celular.

---

## Textos — App Store

**Nome (até 30):** `Altofalante`
**Subtítulo (até 30):** `Seu celular vira caixa de som`

**Texto promocional (até 170):**
`Turbine o som do seu celular: mais alto e mais limpo. Junte vários aparelhos na mesma
Wi-Fi e toque tudo em sincronia — até em estéreo.`

**Descrição:** (use a mesma do Google Play acima)

**Palavras-chave (até 100, separadas por vírgula):**
`som,volume,amplificador,booster,equalizador,caixa,alto-falante,música,grave,bass`

**Categoria:** Música

---

## Privacidade (obrigatório nas duas lojas)
- **Coleta de dados:** nenhuma. O áudio é processado 100% no dispositivo.
- **Permissões:** acesso aos arquivos de música escolhidos pelo usuário; rede local
  (Wi-Fi) para o recurso multi-celular.
- Texto sugerido: _"O Altofalante não coleta nem envia seus dados. O áudio é processado
  apenas no seu aparelho. A rede local é usada só para sincronizar com outros celulares
  que você conectar."_

## Checklist de submissão
- [ ] Ícones gerados (`flutter_launcher_icons`) e splash (`flutter_native_splash`).
- [ ] Screenshots nos tamanhos exigidos.
- [ ] Feature graphic (Play).
- [ ] Política de privacidade publicada (URL) — exigida pelas duas lojas.
- [ ] Build de release assinado (iOS: certificado/perfil; Android: keystore).
