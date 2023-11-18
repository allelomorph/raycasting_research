#include "sdl2_net_smart_ptr.hh"

namespace sdl2_smart_ptr {

namespace deleter {

void SocketSet::operator()(_SDLNet_SocketSet* ssp) const { SDLNet_FreeSocketSet(ssp); }

void TcpSocket::operator()(_TCPsocket* tsp) const { SDLNet_TCP_Close(tsp); }

void UdpPacket::operator()(UDPpacket* upp) const { SDLNet_FreePacket(upp); }

}  // namespace deleter

auto make_unique(_SDLNet_SocketSet* ssp) {
    return unique::SocketSet{ ssp, deleter::SocketSet{} };
}

auto make_unique(_TCPsocket* tsp) {
    return unique::TcpSocket{ tsp, deleter::TcpSocket{} };
}

auto make_unique(UDPpacket* upp) {
    return unique::UdpPacket{ upp, deleter::UdpPacket{} };
}

auto make_shared(_SDLNet_SocketSet* ssp) {
    static deleter::SocketSet dltr;
    return shared::SocketSet{ ssp, dltr };
}

auto make_shared(_TCPsocket* tsp) {
    static deleter::TcpSocket dltr;
    return shared::TcpSocket{ tsp, dltr };
}

auto make_shared(UDPpacket* upp) {
    static deleter::UdpPacket dltr;
    return shared::UdpPacket{ upp, dltr };
}

}  // namespace sdl2_smart_ptr
