/**
 *
 * peer_server.h
 *
 * the peer server
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-03-31
 */

#ifndef __common_rudp_netpeer_peer_server_h__
#define __common_rudp_netpeer_peer_server_h__

#include "peer_server_interface.h"
#include "peer_address.h"
#include "peer_event_handler.h"
#include "peer_session.h"
#include "peer_util.h"
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include <atomic>
#include <map>

namespace RakNet{
    class RakPeerInterface;
    struct SystemAddress;
    struct RakNetGUID;
}

namespace netpeer
{
class peer_session;
class peer_server : public peer_server_interface
{
protected:
    RakNet::RakPeerInterface* rakpeer_interface_;
    std::atomic_bool          stopped_;
    std::atomic_bool          initialized_;
    int32_t                   port_;
    int32_t                   max_connections_;

    typedef std::pair<RakNet::RakNetGUID, RakNet::SystemAddress> SessionKeyType;
    std::map<SessionKeyType, peer_session*> connection_list_;

public:
    peer_server()
    {
        rakpeer_interface_ = RakNet::RakPeerInterface::GetInstance();
        initialized_ = false;
        stopped_ = false;
        port_ = 0;
        max_connections_ = 0;
    }

    virtual ~peer_server()
    {
        stop();
    }

public:
    /** implements from peer_interface */
    virtual void   set_time_out(int32_t milli) override{
        if (rakpeer_interface_){
            rakpeer_interface_->SetTimeoutTime(milli, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
        }
    }

    virtual void   stop() override{
        if (!stopped_.exchange(true)){
            if (rakpeer_interface_){
                rakpeer_interface_->Shutdown(300);
                RakNet::RakPeerInterface::DestroyInstance(rakpeer_interface_);
                rakpeer_interface_ = nullptr;
            }
        }
    }

    void    event_session_closed(RakNet::Packet* p){

        peer_session* session = remove_session(p->guid, p->systemAddress);
        if (session){
            if (event_handler_){
                event_handler_->session_closed(session);
            }
            // free the session
            delete session;
        }
    }

    void    event_new_session(RakNet::Packet* p){
        peer_session* session = new peer_session(rakpeer_interface_, p->systemAddress, p->guid);

        add_session(session);

        if (event_handler_){
            event_handler_->session_opened(session);
        }
        else{
            log_msg(peer_log_level_error,
                "no peer event handler setted, can't fire the session open event of ip[%s].", p->systemAddress.ToString(true));
        }
    }

    void    event_session_receive(RakNet::Packet* p){
        uint32_t data_offset = peer_util::get_userdata_offset(p);
        log_msg(peer_log_level_debug, "recv data from %s with guid:%llu, len[%d].",
            p->systemAddress.ToString(true), p->guid.g, p->length - data_offset);

        peer_session* session = get_session(p->guid, p->systemAddress);
        if (event_handler_ && session){
            event_handler_->session_msg_recv(session, (char*)p->data + data_offset, p->length - data_offset);
        }
    }

    virtual bool   update(float delta_time) override{
        if (!rakpeer_interface_)
            return false;

        bool has_event = false;
        RakNet::Packet* p = nullptr;
        for (p = rakpeer_interface_->Receive(); p; rakpeer_interface_->DeallocatePacket(p), p = rakpeer_interface_->Receive())
        {
            has_event = true;
            unsigned char packet_identifier = peer_util::get_packet_identifier(p);
            switch (packet_identifier)
            {
            case ID_DISCONNECTION_NOTIFICATION:
            {
                log_msg(peer_log_level_info, "ID_DISCONNECTION_NOTIFICATION from %s with guid: %llu.",
                    p->systemAddress.ToString(true), p->guid.g);

                event_session_closed(p);

                break;
            }
            case ID_NEW_INCOMING_CONNECTION:
            {
                log_msg(peer_log_level_info, "accept a new connection from %s with guid %lld.",
                    p->systemAddress.ToString(true), p->guid.g);

                event_new_session(p);

                break;
            }
            case ID_INCOMPATIBLE_PROTOCOL_VERSION:
            {
                log_msg(peer_log_level_fatal, "ID_INCOMPATIBLE_PROTOCOL_VERSION");

                break;
            }
            case ID_CONNECTED_PING:
            case ID_UNCONNECTED_PING:
            {
                log_msg(peer_log_level_debug, "ping from %s.", p->systemAddress.ToString(true));

                break;
            }
            case ID_CONNECTION_LOST:
            {
                log_msg(peer_log_level_info, "connection lost from %s with guid: %llu.",
                    p->systemAddress.ToString(true), p->guid.g);

                event_session_closed(p);

                break;
            }
            case ID_USER_CUSTOM_PACKET:
            {
                event_session_receive(p);

                break;
            }
            default:
            {
                log_msg(peer_log_level_warn, "unknow packet from %s with guid:%llu, identifier[%d:%d], len[%d].",
                    p->systemAddress.ToString(true), p->guid.g, packet_identifier, p->data[0], p->length);
                break;
            }

            }
        }

        return has_event;
    }

    /** implements from peer_server_interface */
    virtual bool   bind(int32_t max_connections, int32_t port) override{
        if (initialized_.exchange(true))
            return true;

        if (!rakpeer_interface_){
            log_msg(peer_log_level_error, "the peer server try bind failed, the rakpeer_interface_ is null");
            return false;
        }

        max_connections_ = max_connections;
        port_ = port;

        RakNet::SocketDescriptor socket_descriptor;
        socket_descriptor.port = port_;
        socket_descriptor.socketFamily = AF_INET;
        RakNet::StartupResult result =
            rakpeer_interface_->Startup(max_connections_, &socket_descriptor, 1);
        if (result != RakNet::RAKNET_STARTED)
        {
            log_msg(peer_log_level_error, "peer_server try start failed, ports[%d], error_code:%d, %s.",
                port, result, peer_util::get_peer_start_error_by_code((int32_t)result));

            return false;
        }
        rakpeer_interface_->SetMaximumIncomingConnections(max_connections_);
        rakpeer_interface_->SetOccasionalPing(true);
        rakpeer_interface_->SetUnreliableTimeout(1000);

        DataStructures::List< RakNet::RakNetSocket2* > sockets;
        rakpeer_interface_->GetSockets(sockets);
        log_msg(peer_log_level_info, "[%p] socket addresses used by peer server:", this);
        for (uint32_t i = 0; i < sockets.Size(); i++)
        {
            log_msg(peer_log_level_info, "[%p] index:%i, %s.", this, i + 1, sockets[i]->GetBoundAddress().ToString(true));
        }

        log_msg(peer_log_level_info, "[%p] the machine ip addresses:", this);
        for (uint32_t i = 0; i < rakpeer_interface_->GetNumberOfAddresses(); i++)
        {
            RakNet::SystemAddress sa = rakpeer_interface_->GetInternalID(RakNet::UNASSIGNED_SYSTEM_ADDRESS, i);
            log_msg(peer_log_level_info, "[%p] index:%i. %s (LAN=%i).", this, i + 1, sa.ToString(false), sa.IsLANAddress());
        }

        log_msg(peer_log_level_info, "[%p] the peer server guid %s.",
            this, rakpeer_interface_->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
        log_msg(peer_log_level_info, "[%p] the peer started, bind port[%d], max_connections[%d].",
            this, port_, max_connections_);

        return true;
    }

protected:
    void           add_session(peer_session* session)
    {
        if (!session)
            return;

        peer_address* paddress = (peer_address*)session->get_address();
        if (!paddress)
            return;

        SessionKeyType key(paddress->rak_guid(), paddress->rak_address());
        auto iter = connection_list_.find(key);
        if (iter == connection_list_.end()){
            connection_list_.insert(std::make_pair(key, session));
            log_msg(peer_log_level_debug,
                "[%p] add a new session %s:%d, guid:%llu, current count[%d].",
                this, paddress->to_string(), paddress->port(), paddress->guid(), connection_list_.size());
        }
        else{
            log_msg(peer_log_level_warn,
                "[%p] try add a new session, %s:%d, guid:%llu, but it already in the map, something is wrong?",
                this, paddress->to_string(), paddress->port(), paddress->guid());
        }
    }

    peer_session*  get_session(const RakNet::RakNetGUID& guid, const RakNet::SystemAddress& addr)
    {
        SessionKeyType key(guid, addr);
        auto iter = connection_list_.find(key);
        if (iter != connection_list_.end()){
            return iter->second;
        }
        return nullptr;
    }

    peer_session*  remove_session(const RakNet::RakNetGUID& guid, const RakNet::SystemAddress& addr)
    {
        peer_session* ret = nullptr;
        SessionKeyType key(guid, addr);
        auto iter = connection_list_.find(key);
        if (iter != connection_list_.end())
        {
            ret = iter->second;
            connection_list_.erase(iter);

            log_msg(peer_log_level_debug,
                "[%p] remove a new session, %s, guid:%llu, remove count[%d]",
                this, addr.ToString(true), guid.g, connection_list_.size());
        }

        return ret;
    }
};

}

#endif