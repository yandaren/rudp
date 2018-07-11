/**
 *
 * peer_server_interface
 *
 * peer server interface
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-03-30
 */

#ifndef __common_rudp_netpeer_server_interface_h__
#define __common_rudp_netpeer_server_interface_h__

#include "peer_interface.h"
#include <stdint.h>

namespace netpeer
{

class np_api peer_server_interface : public peer_interface
{
public:
    virtual ~peer_server_interface(){}

public:
    virtual bool    bind(int32_t max_connections, int32_t port) = 0;
};

}

#endif