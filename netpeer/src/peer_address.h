/**
 *
 * neetpeer.h
 *
 * peer address
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-03-31
 */

#ifndef __common_rudp_netpeer_address_h__
#define __common_rudp_netpeer_address_h__

#include <string>
#include "peer_address_interface.h"
#include "RakNetTypes.h"

namespace netpeer
{
class peer_address : public peer_address_interface
{
protected:
    RakNet::SystemAddress raknet_address_;
    RakNet::RakNetGUID    raknet_guid_;
    std::string           address_ip_;
    int32_t               address_port_;

public:
    peer_address();
    peer_address(const RakNet::SystemAddress& addr, const RakNet::RakNetGUID& guid);
    virtual ~peer_address();

public:
    virtual const char* to_string() override;
    virtual int32_t     port() override;
    virtual uint64_t    guid() override;

    bool operator ==(const peer_address& right) const;
    bool operator !=(const peer_address& right) const;
    bool operator > (const peer_address& right) const;
    bool operator < (const peer_address& right) const;

public:
    const RakNet::SystemAddress& rak_address();
    const RakNet::RakNetGUID&    rak_guid();
};
}

#endif