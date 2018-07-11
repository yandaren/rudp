/**
 *
 * peer_util.h
 *
 * some util of the netpeer
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-03-31
 */

#ifndef __common_rudp_netpeer_peer_util_h__
#define __common_rudp_netpeer_peer_util_h__

#include <stdint.h>

namespace RakNet{
    struct Packet;
}

namespace netpeer
{
class peer_util
{
public:
    static uint32_t     get_indentifier_offset(RakNet::Packet* p);
    static uint8_t      get_packet_identifier(RakNet::Packet* p);
    static uint32_t     get_userdata_offset(RakNet::Packet* p);
    static const char*  get_peer_start_error_by_code(int32_t error_code);
    static const char*  get_peer_connect_error_by_code(int32_t error_code);
};
}

#endif