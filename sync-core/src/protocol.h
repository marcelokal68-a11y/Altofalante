/* Protocolo UDP do sync multi-celular. Pacote unico e simples (prototipo). */
#ifndef ALTOFALANTE_PROTOCOL_H
#define ALTOFALANTE_PROTOCOL_H

#include <cstdint>

namespace af::net {

enum : uint8_t {
    MSG_HELLO       = 1, // seguidor -> lider: "estou aqui, me registre"
    MSG_PROBE       = 2, // seguidor -> lider: sonda de sincronizacao (a = t0)
    MSG_PROBE_REPLY = 3, // lider -> seguidor: a=t0, b=t1(recv), c=t2(send)
    MSG_PLAY        = 4, // lider -> seguidor: a = instante de inicio (relogio do lider)
};

#pragma pack(push, 1)
struct Packet {
    uint8_t  type;
    uint32_t seq;
    double   a, b, c;
};
#pragma pack(pop)

} // namespace af::net

#endif
