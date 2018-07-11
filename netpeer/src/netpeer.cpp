#include <mutex>
#include "netpeer.h"
#include "peer_connector.hpp"
#include "peer_server.hpp"
#include "GetTime.h"
#include "RakNetTypes.h"

namespace netpeer{

    peer_connector_interface* peer_factory::create_peer_connector()
    {
        return new peer_connector();
    }

    void  peer_factory::destory_peer_connector(peer_connector_interface* p)
    {
        if (p)
        {
            delete p;
        }
    }

    peer_server_interface*    peer_factory::create_peer_server()
    {
        return new peer_server();
    }

    void  peer_factory::destory_peer_server(peer_server_interface* p)
    {
        if (p)
        {
            delete p;
        }
    }

    uint32_t  peer_factory::get_send_buffer_pre_bytes()
    {
        return sizeof(RakNet::MessageID);
    }

    uint64_t  peer_factory::get_time_in_nanosecond()
    {
        return RakNet::GetTimeUS();
    }
}