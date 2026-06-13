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
    MSG_ANNOUNCE    = 5, // lider -> grupo multicast: a = porta de dados (descoberta)
};

// Descoberta estilo mDNS (multicast). No prototipo usamos a interface de loopback
// para rodar tudo numa maquina; no produto, troque por interface Wi-Fi.
constexpr const char* MCAST_GROUP = "239.255.42.99";
constexpr uint16_t    MCAST_PORT  = 45200;
constexpr const char* MCAST_IFACE = "127.0.0.1";

#pragma pack(push, 1)
struct Packet {
    uint8_t  type;
    uint32_t seq;
    double   a, b, c;
};
#pragma pack(pop)

} // namespace af::net

#endif
