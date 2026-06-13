# 00 — Visão

## O problema
O alto-falante embutido dos celulares (iPhone e Android) soa **baixo, fino e sem corpo**.
Para ouvir música com qualidade, o usuário precisa carregar uma caixa de som externa
(JBL, Bose, etc.). Isso é um gadget a mais para comprar, carregar e perder.

## A proposta
**Altofalante** é um app que aplica processamento de áudio profissional (DSP) em tempo
real para extrair o **máximo de volume e clareza** que o hardware do celular consegue
entregar — e, quando o usuário tem mais de um aparelho, **sincroniza vários celulares**
para formar um sistema de som maior. O objetivo: chegar o mais perto possível da
experiência de uma caixa portátil **usando só os celulares que as pessoas já têm**.

## Público
- Pessoas que ouvem música/podcast no celular em ambientes informais (cozinha, banho,
  churrasco, viagem) e não têm/não querem carregar uma caixa externa.
- Grupos que querem som mais alto juntando vários celulares (festas pequenas, camping).

## Proposta de valor
1. **Mais alto e mais limpo** que o player nativo, sem distorcer.
2. **Sem hardware extra** — o celular vira a caixa de som.
3. **Multi-celular** — junte aparelhos para mais volume real.
4. **Simples** — abrir e já tocar bem, sem precisar mexer em configurações.

## Estratégia por plataforma (ver `06-estrategia.md`)
- **Android:** turbina **todo** o áudio do sistema (Spotify, YouTube, jogos) via efeitos
  globais — a promessa original cumprida para a maioria. (`07-android-system-audio.md`)
- **iOS:** turbina **suas mídias** tocadas no app (músicas baixadas, podcasts, áudios) —
  a Apple não deixa mexer no som de outros apps.
- **Ambos:** **multi-celular** — junte aparelhos para volume real (o "uau" das festas).

## Limites honestos (o que NÃO prometemos)
- **Não** transformamos o celular numa JBL/Bose de verdade. Um driver minúsculo tem
  limites físicos de SPL e de grave que software nenhum supera.
- **Não** amplificamos o áudio de outros apps no iOS (Spotify, YouTube): a Apple não
  permite. No v1 o áudio toca **dentro** do nosso player.
- O ganho de grave é **psicoacústico** (sensação de grave via harmônicos), não grave
  físico real.

## Métrica de sucesso (norte do produto)
Aumentar o **loudness percebido** (LUFS integrado) do material reproduzido em pelo menos
**+6 LU** em relação ao sinal original, **sem clipping** (true-peak ≤ -1 dBTP), e com
melhora subjetiva de clareza em testes A/B no alto-falante real.
