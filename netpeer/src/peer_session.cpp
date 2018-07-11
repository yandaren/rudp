#include "peer_session.h"
#include "peer_address.h"
#include "netpeer.h"
#include "RakNetTypes.h"
#include "RakPeerInterface.h"
#include "RakNetStatistics.h"
#include "MessageIdentifiers.h"

namespace netpeer{

peer_session::peer_session()
    : peer_address_(nullptr)
    , custom_data_(nullptr)
    , rak_peer_interface_(nullptr)
{
}

peer_session::peer_session(RakNet::RakPeerInterface* pi, const RakNet::SystemAddress& addr, const RakNet::RakNetGUID& guid)
    : custom_data_(nullptr)
    , rak_peer_interface_(pi)
{
    peer_address_ = new peer_address(addr, guid);
}

peer_session::~peer_session()
{
    if (peer_address_){
        delete peer_address_;
        peer_address_ = nullptr;
    }
}

peer_address_interface* peer_session::get_address()
{
    return peer_address_;
}

void    peer_session::set_custom_data(void* data)
{
    custom_data_ = data;
}

void*   peer_session::get_custom_data()
{
    return custom_data_;
}

uint32_t    peer_session::send_msg(const char *data, const int32_t length,
    packet_priority priority, packet_reliability reliability, char ordering_channel)
{
    uint32_t pre_bytes = peer_factory::get_send_buffer_pre_bytes();
    int32_t total_len = pre_bytes + length;
    char* buffer = (char*)malloc(total_len);
    memcpy(&buffer[pre_bytes], data, length);
    uint32_t ret = send_msg_with_prebuffer(&buffer[pre_bytes], length, priority, reliability, ordering_channel);

    free(buffer);

    return ret;
}

uint32_t peer_session::send_msg_with_prebuffer(const char *data, const int32_t length,
    packet_priority priority, packet_reliability reliability, char ordering_channel)
{
    if (!rak_peer_interface_)
        return 0;
    if (!peer_address_)
        return 0;

    uint32_t pre_bytes = peer_factory::get_send_buffer_pre_bytes();
    char* raw_buffer = (char*)(data - pre_bytes);
    raw_buffer[0] = (char)ID_USER_CUSTOM_PACKET;
    int32_t  raw_buffer_len = length + pre_bytes;

    return rak_peer_interface_->Send(raw_buffer, raw_buffer_len, (PacketPriority)priority, (PacketReliability)reliability, ordering_channel, peer_address_->rak_address(), false);
}

void    peer_session::close()
{
    if (!rak_peer_interface_)
        return;
    if (!peer_address_)
        return;

    rak_peer_interface_->CloseConnection(peer_address_->rak_address(), true);
}

bool    peer_session::get_statistics(np_statistics* nps)
{
    if (!rak_peer_interface_)
        return false;
    if (!peer_address_)
        return false;

    RakNet::RakNetStatistics rak_ss;
    if (!rak_peer_interface_->GetStatistics(peer_address_->rak_address(), &rak_ss)){
        return false;
    }

    nps->connection_start_time = rak_ss.connectionStartTime;
    for (int32_t i = 0; i < nps_per_second_metrics_count; ++i){
        nps->last_second[i] = rak_ss.valueOverLastSecond[i];
        nps->running_total[i] = rak_ss.runningTotal[i];
    }

    return true;
}

}