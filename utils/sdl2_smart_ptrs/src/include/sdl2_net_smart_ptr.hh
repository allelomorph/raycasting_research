#ifndef SDL2_NET_SMART_PTR_HH
#define SDL2_NET_SMART_PTR_HH

#include "SDL_net.h"     // _SDLNet_SocketSet _TCPsocket UDPpacket

#include <memory>


namespace sdl2_smart_ptr {

namespace deleter {

/*
 * Note: SDL_net.h, reference version 2.3.0 d585e957d10844823a7269c98a30ffbe370413c6,
 *   typedefs TCPsocket to _TCPsocket* and SDLNet_SocketSet to _SDLNet_SocketSet*,
 *   so using hidden types for consistent syntax with STL smart pointer templates.
 */

struct SocketSet {
    void operator()(_SDLNet_SocketSet*) const;
};

struct TcpSocket {
    void operator()(_TCPsocket*) const;
};

struct UdpPacket {
    void operator()(UDPpacket*) const;
};

}  // namespace deleter

namespace unique {

using SocketSet = std::unique_ptr<_SDLNet_SocketSet, deleter::SocketSet>;
using TcpSocket = std::unique_ptr<_TCPsocket,        deleter::TcpSocket>;
using UdpPacket = std::unique_ptr<UDPpacket,         deleter::UdpPacket>;

}  // namespace unique

namespace shared {

using SocketSet = std::shared_ptr<_SDLNet_SocketSet>;
using TcpSocket = std::shared_ptr<_TCPsocket>;
using UdpPacket = std::shared_ptr<UDPpacket>;

}  // namespace shared

namespace weak {

using SocketSet = std::weak_ptr<_SDLNet_SocketSet>;
using TcpSocket = std::weak_ptr<_TCPsocket>;
using UdpPacket = std::weak_ptr<UDPpacket>;

}  // namespace weak

unique::SocketSet make_unique(_SDLNet_SocketSet*);
unique::TcpSocket make_unique(_TCPsocket*);
unique::UdpPacket make_unique(UDPpacket*);

shared::SocketSet make_shared(_SDLNet_SocketSet*);
shared::TcpSocket make_shared(_TCPsocket*);
shared::UdpPacket make_shared(UDPpacket*);

}  // namespace sdl2_smart_ptr


#endif  // SDL2_NET_SMART_PTR_HH
