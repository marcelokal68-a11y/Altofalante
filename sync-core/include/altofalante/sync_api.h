/*
 * Altofalante sync — API C (estável p/ a ponte nativa iOS/Android).
 *
 * Encapsula descoberta + sincronização de relógio + agendamento. O motor de áudio
 * nativo chama esta API e recebe o INSTANTE MONOTÔNICO (segundos) em que deve iniciar
 * a reprodução, para que todos os aparelhos emitam o som juntos.
 */
#ifndef ALTOFALANTE_SYNC_API_H
#define ALTOFALANTE_SYNC_API_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AfSync AfSync;

/*
 * theta: offset artificial de relógio (apenas para teste/simulação; use 0 em produção).
 * iface: interface de rede do multicast ("" ou NULL = padrão; respeita AF_IFACE).
 */
AfSync* af_sync_create(double theta, const char* iface);
void    af_sync_destroy(AfSync* s);

/* Relógio monotônico compartilhado (segundos) — base do agendamento. */
double af_sync_now(void);

/* ---- LÍDER ---- */
/* Começa a anunciar (multicast) e atender sondas, em thread interna. 0 = ok. */
int    af_sync_start_leader(AfSync* s);
/* Quantos seguidores já se registraram. */
int    af_sync_follower_count(AfSync* s);
/*
 * Dispara um PLAY sincronizado `leadSeconds` no futuro e o envia aos seguidores.
 * Retorna o instante MONOTÔNICO em que o PRÓPRIO líder deve iniciar o áudio
 * (já compensando sua latência de saída).
 */
double af_sync_leader_play(AfSync* s, double leadSeconds, double outputLatency);

/* ---- SEGUIDOR ---- */
/* Descobre o líder e sincroniza o relógio (bloqueante até timeoutMs). 0 = ok. */
int    af_sync_join(AfSync* s, int timeoutMs);
/* Endereço do líder descoberto: preenche `ipOut` e retorna a porta (-1 se não há). */
int    af_sync_leader_endpoint(AfSync* s, char* ipOut, int ipLen);
/* Offset estimado (relógio do líder - do seguidor), em segundos. */
double af_sync_offset(AfSync* s);
/*
 * Espera o próximo PLAY (bloqueante até timeoutMs). Retorna o instante MONOTÔNICO
 * para iniciar o áudio (com compensação de latência), ou valor < 0 em timeout.
 */
double af_sync_wait_play(AfSync* s, double outputLatency, int timeoutMs);

#ifdef __cplusplus
}
#endif

#endif
