#include "peer_address.h"

namespace netpeer{

peer_address::peer_address()
{
    raknet_address_ = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
    raknet_guid_ = RakNet::UNASSIGNED_RAKNET_GUID;
    address_ip_ = "";
    address_port_ = 0;
}

peer_address::peer_address(const RakNet::SystemAddress& addr, const RakNet::RakNetGUID& guid)
{
    raknet_address_ = addr;
    raknet_guid_ = guid;
    address_ip_ = raknet_address_.ToString(false);
    address_port_ = raknet_address_.GetPort();
}

peer_address::~peer_address()
{
}

const char* peer_address::to_string()
{
    return address_ip_.c_str();
}

int32_t     peer_address::port()
{
    return address_port_;
}

uint64_t     peer_address::guid()
{
    return raknet_guid_.g;
}

bool        peer_address::operator == (const peer_address& right) const
{
    return raknet_guid_ == right.raknet_guid_ &&
        raknet_address_ == right.raknet_address_;
}

bool        peer_address::operator != (const peer_address& right) const
{
    return !(*this == right);
}

bool        peer_address::operator > (const peer_address& right) const
{
    if (raknet_guid_ == right.raknet_guid_)
    {
        return raknet_address_ > right.raknet_address_;
    }
    return raknet_guid_ > right.raknet_guid_;
}

bool        peer_address::operator < (const peer_address& right) const
{
    if (raknet_guid_ == right.raknet_guid_)
    {
        return raknet_address_ < right.raknet_address_;
    }
    return raknet_guid_ < right.raknet_guid_;
}

const RakNet::SystemAddress& peer_address::rak_address()
{
    return raknet_address_;
}

const RakNet::RakNetGUID&    peer_address::rak_guid()
{
    return raknet_guid_;
}

}