# 01 — Especificação de produto

## Escopo por versão

### v1 (MVP) — "O celular toca alto e bonito"
- **Player próprio** de arquivos de áudio locais do aparelho (MP3, AAC/M4A, FLAC, WAV).
- **Motor de DSP** aplicado em tempo real na saída (ver `03-dsp-engine.md`).
- **Presets** prontos (1 toque): Voz/Podcast, Música — Equilibrado, Música — Grave+,
  Festa (loudness máximo), e voicing "estilo caixa portátil".
- **Tela única minimalista**: capa/título, play/pause, 1 botão grande de "Turbo"
  (liga/desliga o DSP), seletor de preset. "Simplicidade extrema".
- **Proteção de hardware**: limitador true-peak sempre ativo; aviso ao usar ganho
  extremo por tempo prolongado.

### v2 — "Multi-celular"
- Descoberta de outros aparelhos rodando o app na mesma rede Wi-Fi.
- Sincronização para tocar a mesma faixa em conjunto (mono reforçado ou estéreo L/R).
- Controle a partir de um aparelho "líder".

### v3 — "Inteligência e fontes extras"
- EQ inteligente: ajuste automático de preset por análise da faixa/gênero.
- Equalizador manual avançado (para quem quer ajustar).
- Explorar fontes além do player onde a plataforma permitir (ex.: Android saída de
  áudio; iOS via importação/AirPlay).

## Requisitos funcionais (v1)
- RF1: Listar e tocar arquivos de áudio locais.
- RF2: Aplicar o pipeline de DSP em tempo real, com latência aceitável (< 50 ms).
- RF3: Trocar de preset sem cortar o áudio (transição suave de parâmetros).
- RF4: Ligar/desligar o DSP para comparação A/B instantânea.
- RF5: Persistir o último preset usado.

## Requisitos não-funcionais
- RNF1: **Nunca** produzir clipping digital (true-peak ≤ -1 dBTP na saída).
- RNF2: Núcleo de DSP idêntico em iOS e Android (mesmo C++).
- RNF3: Sem coleta de áudio do usuário; processamento 100% no dispositivo.
- RNF4: Consumo de CPU do DSP baixo o suficiente para não esquentar/drenar bateria
  de forma perceptível em uso normal.

## Fora de escopo (v1)
- Amplificar áudio de outros apps (impossível no iOS).
- Streaming de serviços (Spotify/Apple Music) dentro do app.
- Edição de áudio / gravação.
