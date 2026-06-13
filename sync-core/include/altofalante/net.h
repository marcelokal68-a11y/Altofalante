/* Camada de rede UDP portatil (POSIX sockets) para o sync multi-celular. */
#ifndef ALTOFALANTE_NET_H
#define ALTOFALANTE_NET_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <netinet/in.h>

namespace af::net {

/// Relogio monotonico COMPARTILHADO entre processos da mesma maquina
/// (CLOCK_MONOTONIC). Em segundos. Base da medicao de sincronizacao.
double monotonicNow();

struct Addr {
    sockaddr_in sa{};
    bool operator==(const Addr& o) const;
};

class UdpSocket {
public:
    ~UdpSocket();
    bool open();
    bool bindPort(uint16_t port);          // 0 = porta efemera
    void close();

    bool sendTo(const void* data, size_t len, const Addr& to);
    /// Retorna bytes recebidos, 0 em timeout, -1 em erro. Preenche `from`.
    int  recv(void* buf, size_t len, Addr& from, int timeoutMs);

    uint16_t localPort();              // porta efetivamente vinculada

    // --- multicast (descoberta) ---
    bool joinMulticast(const std::string& group, uint16_t port,
                       const std::string& iface);  // listener
    bool setMulticastIface(const std::string& iface); // sender
    bool enableMulticastLoop();

    static Addr makeAddr(const std::string& host, uint16_t port);

private:
    int fd_ = -1;
};

} // namespace af::net

#endif
