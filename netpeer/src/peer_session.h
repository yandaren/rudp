/**
 *
 * peer_sesion.h
 *
 * the client of the peer session
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-03-31
 */

#ifndef __common_rudp_netpeer_peer_session_h__
#define __common_rudp_netpeer_peer_session_h__

#include "peer_session_interface.h"

namespace RakNet{
    class RakPeerInterface;
    struct SystemAddress;
    struct RakNetGUID;
}

namespace netpeer
{
class peer_address;
class peer_session : public peer_session_interface
{
protected:
    peer_address*               peer_address_;
    void*                       custom_data_;
    RakNet::RakPeerInterface*   rak_peer_interface_;

public:
    peer_session();
    peer_session(RakNet::RakPeerInterface* pi, const RakNet::SystemAddress& addr, const RakNet::RakNetGUID& guid);
    virtual ~peer_session();

public:

    /** implements of peer_session_interface */
    virtual peer_address_interface* get_address() override;
    virtual void    set_custom_data(void* data) override;
    virtual void*   get_custom_data() override;
    virtual uint32_t send_msg(const char *data, const int32_t length,
        packet_priority priority, packet_reliability reliability, char ordering_channel) override;
    virtual uint32_t send_msg_with_prebuffer(const char *data, const int32_t length,
        packet_priority priority, packet_reliability reliability, char ordering_channel) override;
    virtual void    close() override;
    virtual bool    get_statistics(np_statistics* nps) override;
};
}

#endif