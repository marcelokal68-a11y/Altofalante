# 05 — Roadmap

## Fase 1 — Núcleo de DSP + prova de valor  ◀ ATUAL
**Meta:** provar, de forma medível e sem device, que o DSP deixa o som mais alto e
limpo sem clipping.

Entregáveis:
- [x] specs/
- [x] `dsp-core` em C++: pipeline completo (gain, EQ, multiband comp, exciter, stereo,
      limiter) + presets.
- [x] `tools/process_wav` (CLI) e `tools/measure.py` (LUFS + true-peak).
- [x] Geração de amostra de teste + relatório before/after.
- [x] Testes de invariantes (limitador nunca estoura; loudness sobe; passthrough).

**Critério de saída:** relatório mostra ganho de loudness vs entrada **e**
true-peak ≤ -1 dBTP; testes passam.

**Resultado medido** (amostra já-alta de -15.3 LUFS; rode `bash tools/demo.sh`):

| Preset    | LUFS   | True-peak | ΔLUFS |
|-----------|--------|-----------|-------|
| voice     | -10.2  | -1.8 dBTP | +5.1  |
| balanced  | -10.0  | -1.5 dBTP | +5.3  |
| bass      |  -9.5  | -1.4 dBTP | +5.8  |
| party     | -10.3  | -1.4 dBTP | +5.0  |
| portable  |  -8.5  | -1.4 dBTP | +6.8  |

Ganho de +5 a +6.8 LU **sem clipping** (todos ≤ -1 dBTP). Os 3 testes de invariante
passam. ✅ Prova de valor da Fase 1 concluída.

## Fase 2 — Multi-celular  ◀ PROTÓTIPO CONCLUÍDO
- [x] Estimativa de offset de relógio estilo NTP (filtro de menor atraso) — `sync-core/`.
- [x] Agendador de início com compensação de latência de saída.
- [x] Simulação medindo defasagem entre 4 nós (relógios ±50 s + jitter assimétrico).
- [x] **Transporte real sobre UDP** (`sync-core/net.cpp` + `sync_node`): sondagem de
      relógio e comando de PLAY por sockets reais; medição com 3 nós no localhost.
- [ ] Descoberta na rede (mDNS) — hoje o endereço do líder é explícito; próximo passo.

**Resultados medidos:**
- Simulação (`./build/sync-core/sync_sim`): defasagem **3.06 ms** com jitter.
- Rede UDP real (`bash tools/sync_net_test.sh`): defasagem **0.011 ms** entre 3 nós
  com relógios dessincronizados em -7/+12.5/+33.2 s e latências distintas. ✅

**Critério de saída (no device):** dois aparelhos físicos tocam em sincronia com
defasagem < 10 ms em LAN.

## Fase 3 — App Flutter (no device)  ◀ ESQUELETO CONCLUÍDO
- [x] Esqueleto do app: tela única (Turbo + presets + play), cliente do MethodChannel
      (`app/lib/`), persistência de preset.
- [ ] Gerar pastas nativas (`flutter create .`).
- [ ] Cabear `dsp-core` via AVAudioEngine (iOS) e Oboe (Android) no MethodChannel.
- [ ] Integração do módulo de sync.
**Critério de saída:** build instalável; teste A/B audível no speaker real; sync entre
2 aparelhos físicos.

## Fase 4 — Polimento e lançamento
- EQ inteligente por gênero, ajuste fino de presets com escuta, telemetria opcional,
  publicação nas lojas.

## Verificação contínua
Cada mudança no `dsp-core` deve passar pelo `measure.py` e pelos testes de invariante
antes do commit. Os números dos presets são calibrados por esse loop de medição.
