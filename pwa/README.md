# Altofalante — PWA (app web instalável)

Versão web do Altofalante: roda no navegador do celular, **instala na tela de início**,
funciona offline e processa o áudio com a Web Audio API (mesmo pipeline conceitual do
motor C++: ganho → EQ → compressão → exciter → limitador). **Não precisa de Mac, conta
de loja nem aprovação.**

> Limite honesto: o recurso **multi-celular** (tocar junto) **não** existe na PWA — o
> navegador não permite a rede UDP/multicast usada para isso. Esse diferencial fica no
> app nativo. A PWA entrega o **player turbinado single-device**.

## Testar no seu computador
A PWA precisa de HTTPS **ou** localhost (por causa do service worker):
```bash
cd pwa
python3 -m http.server 8000
# abra http://localhost:8000 no navegador
```

## Publicar de graça (GitHub Pages)
1. No GitHub, em **Settings → Pages**, ative o Pages a partir da branch (pasta `/pwa`),
   ou copie o conteúdo de `pwa/` para um repositório/branch `gh-pages`.
2. A URL gerada (https://...) já serve a PWA com HTTPS.
3. No celular, abra a URL → menu do navegador → **"Adicionar à tela de início"**.

(Outras opções de hospedagem gratуita com HTTPS: Netlify, Vercel, Cloudflare Pages —
basta apontar para a pasta `pwa/`.)

## Como usar
1. Toque no topo para **escolher músicas** (uma ou várias = fila).
2. Botão **TURBO** liga/desliga o processamento.
3. **✨ Auto** analisa a faixa e escolhe o modo; ou toque um modo manualmente.
4. **▶ ⏮ ⏭** controlam a reprodução.

## Arquivos
- `index.html` — app + motor de áudio (Web Audio).
- `manifest.webmanifest` — nome, ícones, cor (torna instalável).
- `sw.js` — service worker (offline / instalação).
- `icons/` — ícones 192 e 512.
