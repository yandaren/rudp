/**
 *
 * peer_connector.h
 *
 * the client of the peer session
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-03-31
 */

#ifndef __common_rudp_netpeer_connector_h__
#define __common_rudp_netpeer_connector_h__

#include "peer_connector_interface.h"
#include "peer_session.h"
#include "peer_address.h"
#include "peer_util.h"
#include "peer_event_handler.h"
#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include <atomic>

namespace netpeer
{
class peer_connector : 
    public peer_connector_interface
{
protected:
    RakNet::RakPeerInterface* rakpeer_interface_;
    peer_session*             peer_session_;
    std::atomic_bool          stopped_;
    std::atomic_bool          initialized_;

public:
    peer_connector()
    {
        rakpeer_interface_ = RakNet::RakPeerInterface::GetInstance();
        initialized_ = false;
        stopped_ = false;
        peer_session_ = nullptr;
    }
    virtual ~peer_connector()
    {
        stop();

        if (peer_session_){
            delete peer_session_;
            peer_session_ = nullptr;
        }
    }

public:

    /** implements from peer_interface */
    virtual void   set_time_out(int32_t milli) override
    {
        if (rakpeer_interface_){
            rakpeer_interface_->SetTimeoutTime(milli, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
        }
    }

    virtual void   stop() override
    {
        if (!stopped_.exchange(true)){
            if (rakpeer_interface_){
                rakpeer_interface_->Shutdown(300);
                RakNet::RakPeerInterface::DestroyInstance(rakpeer_interface_);
                rakpeer_interface_ = nullptr;
            }
        }
    }

    void event_session_closed(RakNet::Packet* p){
        // locked
        if (peer_session_){
            peer_address* paddress = (peer_address*)peer_session_->get_address();
            if (paddress && paddress->rak_address() == p->systemAddress && paddress->rak_guid() == p->guid){
                if ( event_handler_ )
                    event_handler_->session_closed(peer_session_);

                delete peer_session_;
                peer_session_ = nullptr;
            }
        }
    }

    void event_session_opened(RakNet::Packet* p){
        // This tells the client they have connected
        if (peer_session_){
            delete peer_session_;
            peer_session_ = nullptr;
        }

        peer_session_ = new peer_session(rakpeer_interface_, p->systemAddress, p->guid);
        if (event_handler_){
            event_handler_->session_opened(peer_session_);
        }
    }

    void event_session_receive(RakNet::Packet* p){
        uint32_t data_offset = peer_util::get_userdata_offset(p);

        log_msg(peer_log_level_debug, "recv data from %s with guid:%llu, len[%d].",
            p->systemAddress.ToString(true), p->guid.g, p->length - data_offset);

        if (event_handler_ && peer_session_){
            event_handler_->session_msg_recv(peer_session_, (char*)p->data + data_offset, p->length - data_offset);
        }
    }

    virtual bool   update(float delta_time) override
    {
        if (!rakpeer_interface_)
            return false;

        bool has_event = false;
        RakNet::Packet* p = nullptr;
        for (p = rakpeer_interface_->Receive(); p; rakpeer_interface_->DeallocatePacket(p), p = rakpeer_interface_->Receive())
        {
            has_event = true;
            uint8_t packet_indentifier = peer_util::get_packet_identifier(p);
            switch (packet_indentifier)
            {
            case ID_DISCONNECTION_NOTIFICATION:
            {
                log_msg(peer_log_level_error, "ID_DISCONNECTION_NOTIFICATION from %s, guid: %llu.",
                    p->systemAddress.ToString(true), p->guid.g);

                event_session_closed(p);

                break;
            }
            case ID_ALREADY_CONNECTED:
            {
                log_msg(peer_log_level_warn, "ID_ALREADY_CONNECTED to %s with guid %llu", 
                    p->systemAddress.ToString(true), p->guid.g);

                break;
            }
            case ID_INCOMPATIBLE_PROTOCOL_VERSION:
            {
                log_msg(peer_log_level_fatal, "ID_INCOMPATIBLE_PROTOCOL_VERSION.");

                break;
            }
            case ID_REMOTE_DISCONNECTION_NOTIFICATION:
            {
                log_msg(peer_log_level_debug, "ID_REMOTE_DISCONNECTION_NOTIFICATION");

                break;
            }
            case ID_REMOTE_CONNECTION_LOST:
            {
                log_msg(peer_log_level_debug, "ID_REMOTE_CONNECTION_LOST");

                break;
            }
            case ID_REMOTE_NEW_INCOMING_CONNECTION:
            {
                log_msg(peer_log_level_debug, "ID_REMOTE_NEW_INCOMING_CONNECTION");

                break;
            }
            case ID_CONNECTION_BANNED:
            {
                log_msg(peer_log_level_error, "we are banned from the server %s.", 
                    p->systemAddress.ToString(true));

                break;
            }
            case ID_CONNECTION_ATTEMPT_FAILED:
            {
                log_msg(peer_log_level_error, "connection attempt failed to %s.", 
                    p->systemAddress.ToString(true));

                break;
            }
            case ID_NO_FREE_INCOMING_CONNECTIONS:
            {
                log_msg(peer_log_level_error, "the server is not accepting new connections, %s.", 
                    p->systemAddress.ToString(true));

                break;
            }
            case ID_INVALID_PASSWORD:
            {
                log_msg(peer_log_level_error, "invalid passwd to %s.", 
                    p->systemAddress.ToString(true));

                break;
            }
            case ID_CONNECTION_LOST:
            {
                log_msg(peer_log_level_error, "connection to %s lost, guid: %llu.", 
                    p->systemAddress.ToString(true), p->guid.g);

                event_session_closed(p);

                break;
            }
            case ID_CONNECTION_REQUEST_ACCEPTED:
            {
                log_msg(peer_log_level_info, "connection to %s accepted with guid: %llu.", 
                    p->systemAddress.ToString(true), p->guid.g);
                log_msg(peer_log_level_info, "my external address is %s.", 
                    rakpeer_interface_->GetExternalID(p->systemAddress).ToString(true));

                event_session_opened(p);

                break;
            }
            case ID_CONNECTED_PING:
            case ID_UNCONNECTED_PING:
            {
                log_msg(peer_log_level_debug, "ping from %s.", p->systemAddress.ToString(true));

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
                    p->systemAddress.ToString(true), p->guid.g, packet_indentifier, p->data[0], p->length);

                break;
            }
            }
        }

        return has_event;
    }

    /** implemetns from peer_connector_interface */
    virtual bool   connect(const char* ip, int32_t port) override
    {
        if (!initialize()){
            return false;
        }

        if (!rakpeer_interface_){
            log_msg(peer_log_level_error,
                "try connect remote address[%s:%d] failed, but rakpeer interface is null",
                ip, port);

            return false;
        }

        RakNet::ConnectionAttemptResult result = rakpeer_interface_->Connect(ip, port, nullptr, 0);
        if (result != RakNet::CONNECTION_ATTEMPT_STARTED){
            log_msg(peer_log_level_error,
                "try connect remote address[%s:%d] failed, error_code:%d, %s",
                ip, port, result, peer_util::get_peer_connect_error_by_code((int32_t)result));

            return false;
        }
        return true;
    }

    virtual bool   is_connected() override
    {
        return peer_session_ != nullptr;
    }

    virtual void   disconnect() override
    {
        if (rakpeer_interface_ && peer_session_){
            peer_address* address = (peer_address*)peer_session_->get_address();
            if (address){
                rakpeer_interface_->CloseConnection(address->rak_address(), true);
            }
        }
    }

    virtual uint32_t send_msg(const char *data, const int32_t length,
        packet_priority priority, packet_reliability reliability, char ordering_channel) override
    {
        if (peer_session_){
            return peer_session_->send_msg(data, length, priority, reliability, ordering_channel);
        }
        else{
            log_msg(peer_log_level_warn, "connection not connected, send msg failed");
            return 0;
        }
    }

    virtual uint32_t send_msg_with_prebuffer(const char *data, const int32_t length,
        packet_priority priority, packet_reliability reliability, char ordering_channel) override
    {
        if (peer_session_){
            return peer_session_->send_msg_with_prebuffer(data, length, priority, reliability, ordering_channel);
        }
        else{
            log_msg(peer_log_level_warn, "connection not connected, send_msg_with_prebufferg failed");
            return 0;
        }
    }

private:
    bool           initialize()
    {
        if (!initialized_.exchange(true)){
            rakpeer_interface_->AllowConnectionResponseIPMigration(false);
            RakNet::SocketDescriptor socket_descriptor;
            socket_descriptor.socketFamily = AF_INET;
            RakNet::StartupResult ret = rakpeer_interface_->Startup(1, &socket_descriptor, 1);
            if (ret != RakNet::RAKNET_STARTED)
            {
                log_msg(peer_log_level_fatal, "start up the socket failed, error:%d, %s",
                    ret, peer_util::get_peer_start_error_by_code((int32_t)ret));

                return false;
            }
            rakpeer_interface_->SetOccasionalPing(true);
        }
        return true;
    }
};

}

#endif