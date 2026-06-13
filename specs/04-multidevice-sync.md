# 04 — Multi-celular sincronizado (Fase 2)

> Esta spec é de projeto; a implementação vem depois do MVP (Fase 1).

## Objetivo
Fazer 2+ celulares na mesma rede Wi-Fi tocarem a mesma faixa **em sincronia**, formando
um sistema de som maior. É o único caminho para aumentar o volume **físico real**
(mais alto-falantes), em vez de só perceptivo.

## Modelos de uso
- **Mono reforçado:** todos tocam o mesmo conteúdo (mais alto, mesma faixa).
- **Estéreo:** um aparelho = canal L, outro = canal R.
- (Futuro) **Multiroom:** grupos independentes.

## Desafios técnicos e abordagem
1. **Descoberta:** mDNS/Bonjour (`_altofalante._udp`) anuncia/encontra aparelhos.
2. **Papéis:** um aparelho é **líder** (controla transporte e clock); demais são
   **seguidores**.
3. **Sincronização de relógio:** protocolo tipo NTP/PTP sobre a LAN para estimar o
   offset de relógio entre líder e seguidores (alvo: erro < ~1 ms; acima de ~10 ms já
   gera eco perceptível).
4. **Transporte de mídia:** duas opções a avaliar —
   - (a) cada aparelho já tem o arquivo → líder envia só comandos de play + timestamp
     alvo ("toque o sample N no tempo T"); mais simples e robusto.
   - (b) streaming do líder via UDP + buffer de jitter; necessário se só o líder tem a
     faixa.
   Decisão provável: começar por (a).
5. **Compensação de latência:** medir/parametrizar latência de saída de cada modelo de
   device; ajustar o instante de início por aparelho.
6. **Deriva de clock:** corrigir drift ao longo da faixa (micro-ajustes de resampling
   ou skip/insert imperceptível).

## Critérios de sucesso
- Defasagem entre aparelhos < 10 ms (sem eco audível).
- Reentrada de um aparelho atrasado sem reiniciar os demais.
- Funciona com aparelhos de marcas/modelos diferentes.

## Riscos
- iOS restringe execução em background e timers de alta precisão.
- Variabilidade enorme de latência de áudio entre modelos Android.
- Wi-Fi congestionado degrada a sincronização.

## Estado da implementação (protótipo)
- `sync-core/`: estimativa de offset (`clock_sync.cpp`), agendador com compensação de
  latência (`scheduler.cpp`) e camada UDP POSIX (`net.cpp`) — portátil p/ iOS/Android.
- `sync_node` roda como **líder** ou **seguidor** e troca sondagem + PLAY por UDP real.
- Medição local (`tools/sync_net_test.sh`): 3 nós sincronizam com defasagem ~0.01 ms.
- **Falta para o device:** descoberta mDNS (hoje o endereço do líder é passado
  explicitamente) e o transporte de mídia (modelo (a): cada aparelho já tem a faixa e
  o líder só envia o instante de início).
- _Nota de portabilidade:_ os pacotes enviam `double` cru (ok p/ mesma arquitetura no
  protótipo); no produto, serializar em ordem de rede.
