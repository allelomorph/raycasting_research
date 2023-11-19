#include "sdl2_net_smart_ptr.hh"

namespace sdl2_smart_ptr {

namespace deleter {

void SocketSet::operator()(_SDLNet_SocketSet* ssp) const { SDLNet_FreeSocketSet(ssp); }

void TcpSocket::operator()(_TCPsocket* tsp) const { SDLNet_TCP_Close(tsp); }

void UdpPacket::operator()(UDPpacket* upp) const { SDLNet_FreePacket(upp); }

}  // namespace deleter

unique::SocketSet make_unique(_SDLNet_SocketSet* ssp) {
    static const deleter::SocketSet dltr;
    return unique::SocketSet{ ssp, dltr };
}

unique::TcpSocket make_unique(_TCPsocket* tsp) {
    static const deleter::TcpSocket dltr;
    return unique::TcpSocket{ tsp, dltr };
}

unique::UdpPacket make_unique(UDPpacket* upp) {
    static const deleter::UdpPacket dltr;
    return unique::UdpPacket{ upp, dltr };
}

shared::SocketSet make_shared(_SDLNet_SocketSet* ssp) {
    static const deleter::SocketSet dltr;
    return shared::SocketSet{ ssp, dltr };
}

shared::TcpSocket make_shared(_TCPsocket* tsp) {
    static const deleter::TcpSocket dltr;
    return shared::TcpSocket{ tsp, dltr };
}

shared::UdpPacket make_shared(UDPpacket* upp) {
    static const deleter::UdpPacket dltr;
    return shared::UdpPacket{ upp, dltr };
}

}  // namespace sdl2_smart_ptr
