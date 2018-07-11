#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "RakNetStatistics.h"
#include "RakNetTypes.h"
#include "BitStream.h"
#include "RakSleep.h"
#include "PacketLogger.h"
#include <assert.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <iostream>
#include <memory>
#include <time.h>
#include "netpeer.h"

using namespace std;

unsigned int GetIdentifierOffset(RakNet::Packet *p)
{
    if ((unsigned char)p->data[0] == ID_TIMESTAMP)
    {
        RakAssert(p->length > sizeof(RakNet::MessageID) + sizeof(RakNet::Time));
        return sizeof(RakNet::MessageID) + sizeof(RakNet::Time);
    }
    else
        return 0;
}

unsigned char GetPacketIdentifier(RakNet::Packet *p)
{
    if (p == 0)
        return 255;

    return (unsigned char)p->data[GetIdentifierOffset(p)];
}

unsigned int GetUserDataOffset(RakNet::Packet *p)
{
    return GetIdentifierOffset(p) + sizeof(RakNet::MessageID);
}

void server_test()
{
    int port;
    std::cout << "input port:" << std::endl;
    std::cin >> port;
    printf("starting server\n");
    RakNet::SocketDescriptor socket_descriptor;
    socket_descriptor.port = port;
    socket_descriptor.socketFamily = AF_INET;
    RakNet::RakPeerInterface* server = RakNet::RakPeerInterface::GetInstance();
    const char* passwd = "helloworld";
    //server->SetIncomingPassword(passwd, strlen(passwd));
    server->SetTimeoutTime(30000, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
    int max_connections = 100;
    if (server->Startup(max_connections, &socket_descriptor, 1) != RakNet::RAKNET_STARTED)
    {
        printf("failed to start dual ipv4 ports[%d].\n", port);
        system("pause");
        return;
    }
    server->SetMaximumIncomingConnections(max_connections);
    server->SetOccasionalPing(true);
    server->SetUnreliableTimeout(1000);

    DataStructures::List< RakNet::RakNetSocket2* > sockets;
    server->GetSockets(sockets);
    printf("Socket addresses used by RakNet:\n");
    for (unsigned int i = 0; i < sockets.Size(); i++)
    {
        printf("%i. %s\n", i + 1, sockets[i]->GetBoundAddress().ToString(true));
    }

    printf("\nMy IP addresses:\n");
    for (unsigned int i = 0; i < server->GetNumberOfAddresses(); i++)
    {
        RakNet::SystemAddress sa = server->GetInternalID(RakNet::UNASSIGNED_SYSTEM_ADDRESS, i);
        printf("%i. %s (LAN=%i)\n", i + 1, sa.ToString(false), sa.IsLANAddress());
    }

    printf("\nMy GUID is %s\n", server->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());

    while (true){

        RakNet::Packet* p = nullptr;
        for (p = server->Receive(); p; server->DeallocatePacket(p), p = server->Receive())
        {
            unsigned char packetIdentifier = GetPacketIdentifier(p);

            switch (packetIdentifier)
            {
            case ID_DISCONNECTION_NOTIFICATION:
            {
                printf("ID_DISCONNECTION_NOTIFICATION from %s\n", p->systemAddress.ToString(true));
                break;
            }
            case ID_NEW_INCOMING_CONNECTION:
            {
                printf("ID_NEW_INCOMING_CONNECTION from %s with guid %s\n", p->systemAddress.ToString(true), p->guid.ToString());

                RakNet::SystemAddress client_id = p->systemAddress;
                for (int index = 0; index < MAXIMUM_NUMBER_OF_INTERNAL_IDS; index++)
                {
                    RakNet::SystemAddress internal_id = server->GetInternalID(p->systemAddress, index);
                    if (internal_id != RakNet::UNASSIGNED_SYSTEM_ADDRESS)
                    {
                        printf("%i, %s\n", index + 1, internal_id.ToString(true));
                    }
                }

                break;
            }
            case ID_INCOMPATIBLE_PROTOCOL_VERSION:
            {
                printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
                break;
            }
            case ID_CONNECTED_PING:
            case ID_UNCONNECTED_PING:
            {
                printf("Ping from %s\n", p->systemAddress.ToString(true));
                break;
            }
            case ID_CONNECTION_LOST:
            {
                printf("ID_CONNECTION_LOST from %s\n", p->systemAddress.ToString(true));
                break;
            }
            case ID_USER_CUSTOM_PACKET:
            {
                unsigned int data_offset = GetUserDataOffset(p);
                std::string data((char*)p->data + data_offset, p->length - data_offset);
                printf("recv data %s, size[%d] from address %s\n", data.c_str(), data.size(), p->systemAddress.ToString(true));
                char send_data[1024] = { 0 };
                send_data[0] = (char)ID_USER_CUSTOM_PACKET;
                memcpy(&send_data[1], data.c_str(), data.size());
                server->Send(send_data, data.size() + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, false);
                break;
            }
            default:
            {
                printf("unknow packet, identifier[%d:%d], len[%d].", packetIdentifier, p->data[0], p->length);
                break;
            }

            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    server->Shutdown(300);
    RakNet::RakPeerInterface::DestroyInstance(server);
}

void client_test()
{
    std::cout << "input the remote ip and port:" << std::endl;
    std::string ip;
    int port;
    std::cin >> ip >> port;
    
    RakNet::RakPeerInterface* client = RakNet::RakPeerInterface::GetInstance();
    client->AllowConnectionResponseIPMigration(false);
    RakNet::SystemAddress clientID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

    RakNet::SocketDescriptor socket_descriptor;
    socket_descriptor.socketFamily = AF_INET;
    RakNet::StartupResult ret = client->Startup(1, &socket_descriptor, 1);
    if (ret != RakNet::RAKNET_STARTED)
    {
        printf("start up the socket failed, error:%s\n", ret);
        return;
    }
    client->SetOccasionalPing(true);
    const char* passwd = "helloworld";
    RakNet::ConnectionAttemptResult cret = client->Connect(ip.c_str(), port, nullptr, 0);
    if (cret != RakNet::CONNECTION_ATTEMPT_STARTED)
    {
        printf("try connect server %s:%d failed, error:%d\n", ip.c_str(), port, cret);
        return;
    }

    printf("\nMy IP addresses:\n");
    unsigned int i;
    for (i = 0; i < client->GetNumberOfAddresses(); i++)
    {
        printf("%i. %s\n", i + 1, client->GetLocalIP(i));
    }

    printf("My GUID is %s\n", client->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());

    time_t last = time(nullptr);

    RakNet::SystemAddress remoteAddress;
    bool connected = false;
    while (true){

        time_t now = time(nullptr);
        if (now - last > 5 && connected){
            printf("input msg:\n");
            std::string msg;
            std::cin >> msg;
            char send_data[1024] = { 0 };
            send_data[0] = (char)ID_USER_CUSTOM_PACKET;
            memcpy(&send_data[1], msg.c_str(), msg.size());
            client->Send(send_data, msg.size() + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, remoteAddress, false);
            last = now;
        }

        RakNet::Packet* p = nullptr;
        for (p = client->Receive(); p; client->DeallocatePacket(p), p = client->Receive())
        {
            unsigned char packetIdentifier = GetPacketIdentifier(p);
            switch (packetIdentifier)
            {
            case ID_DISCONNECTION_NOTIFICATION:
            {
                printf("ID_DISCONNECTION_NOTIFICATION\n");
                break;
            }
            case ID_ALREADY_CONNECTED:
            {
                printf("ID_ALREADY_CONNECTED with guid %s\n", p->guid.ToString());
                break;
            }
            case ID_INCOMPATIBLE_PROTOCOL_VERSION:
            {
                printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
                break;
            }
            case ID_REMOTE_DISCONNECTION_NOTIFICATION:
            {
                printf("ID_REMOTE_DISCONNECTION_NOTIFICATION\n");
                break;
            }
            case ID_REMOTE_CONNECTION_LOST:
            {
                printf("ID_REMOTE_CONNECTION_LOST\n");
                break;
            }
            case ID_REMOTE_NEW_INCOMING_CONNECTION:
            {
                printf("ID_REMOTE_NEW_INCOMING_CONNECTION\n");
                break;
            }
            case ID_CONNECTION_BANNED:
            {
                printf("we are banned from this server.\n");
                break;
            }
            case ID_CONNECTION_ATTEMPT_FAILED:
            {
                printf("connection attempt failed\n");
                break;
            }
            case ID_NO_FREE_INCOMING_CONNECTIONS:
            {
                printf("ID_NO_FREE_INCOMING_CONNECTIONS\n");
                break;
            }
            case ID_INVALID_PASSWORD:
            {
                printf("invalid passwd\n");
                break;
            }
            case ID_CONNECTION_LOST:
            {
                printf("ID_CONNECTION_LOST, %s\n", p->systemAddress.ToString(true));
                break;
            }
            case ID_CONNECTION_REQUEST_ACCEPTED:
            {
                // This tells the client they have connected
                remoteAddress = p->systemAddress;
                connected = true;
                printf("ID_CONNECTION_REQUEST_ACCEPTED to %s with GUID %s\n", p->systemAddress.ToString(true), p->guid.ToString());
                printf("My external address is %s\n", client->GetExternalID(p->systemAddress).ToString(true));
                break;
            }
            case ID_CONNECTED_PING:
            case ID_UNCONNECTED_PING:
            {
                printf("Ping from %s\n", p->systemAddress.ToString(true));
                break;
            }
            case ID_USER_CUSTOM_PACKET:
            {
                unsigned int data_offset = GetUserDataOffset(p);
                std::string data((char*)p->data + data_offset, p->length - data_offset);
                printf("recv data %s, size[%d] from address %s\n", data.c_str(), data.size(), p->systemAddress.ToString(true));

                break;
            }
            default:
            {
                printf("unknow packet, identifier[%d:%d], len[%d].", packetIdentifier, p->data[0], p->length);
                break;
            }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    client->Shutdown(300);
    RakNet::RakPeerInterface::DestroyInstance(client);
}

static const char* get_log_lvl_str(netpeer::peer_log_level lvl){
    static const char* log_level_strs[] = { "debug", "info", "warn", "error", "fatal" };
    static const char* uknow_level = "unknow";
    if (lvl >= netpeer::peer_log_level_debug &&
        lvl <= netpeer::peer_log_level_fatal){
        return log_level_strs[lvl];
    }
    return uknow_level;
}

class client_event_handler : public netpeer::peer_event_handler
{
public:
    client_event_handler()
    {

    }

    ~client_event_handler()
    {

    }

public:

    /** 一些连接事件 */
    virtual void    session_opened(netpeer::peer_session_interface* session) override
    {
        printf("session guid[%lld] to address[%s:%d] opened\n",
            session->get_address()->guid(),
            session->get_address()->to_string(), session->get_address()->port());
    }

    virtual void    session_closed(netpeer::peer_session_interface* session) override
    {
        printf("session guid[%lld] to address[%s:%d] closed\n",
            session->get_address()->guid(),
            session->get_address()->to_string(), session->get_address()->port());
    }

    virtual void    session_msg_recv(netpeer::peer_session_interface* session, char* data, int32_t len) override
    {
        char str[1024] = { 0 };
        memcpy(str, data, len);

        printf("recv msg[%s] len[%d] from session guid[%lld] to address[%s:%d]\n",
            str, len,
            session->get_address()->guid(),
            session->get_address()->to_string(), session->get_address()->port());
    }

    /** 一些其他事件 */
    virtual void    log_msg(netpeer::peer_log_level log_level, char* data, int32_t len) override
    {
        printf("[netpper] [%s] %s.\n", get_log_lvl_str(log_level), data);
    }
};

void client_test1(){

    std::cout << "input the remote ip and port:" << std::endl;
    std::string ip;
    int port;
    std::cin >> ip >> port;

    netpeer::peer_connector_interface* connector = netpeer::peer_factory::create_peer_connector_mt();
    connector->set_event_handler(new client_event_handler());
    connector->set_log_level(netpeer::peer_log_level_info);
    if (!connector->connect(ip.c_str(), port)){
        printf("try connect server[%s:%d] failed\n", ip.c_str(), port);
        return;
    }
    connector->set_time_out(10000);

    std::thread tick_thread([=](){
        printf("tick thread start run.\n");
        while (true){
            if (!connector->update(10)){
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    });

    uint32_t pre_bytes = netpeer::peer_factory::get_send_buffer_pre_bytes();
    std::string msg;
    int32_t count = 0;
    while (true){
        std::cout << "input msg:";
        std::cin >> msg;
        char send_data[1024] = { 0 };
        memcpy(&send_data[pre_bytes], msg.c_str(), msg.size());
        connector->send_msg_with_prebuffer(&send_data[pre_bytes], msg.size(), netpeer::priority_high, netpeer::reliable_ordered, 0);
        //connector->send_msg(msg.c_str(), msg.size(), netpeer::priority_high, netpeer::reliable_ordered, 0);

        count++;
        if (count % 3 == 0){
            connector->disconnect();
        }
    }

    netpeer::peer_factory::destory_peer_connector(connector);
}

class server_event_handler : public netpeer::peer_event_handler
{
public:
    server_event_handler()
    {
    }

    ~server_event_handler()
    {
    }

public:

    /** 一些连接事件 */
    virtual void    session_opened(netpeer::peer_session_interface* session) override
    {
        printf("a new connection %s:%d, guid:%llu opened\n",
            session->get_address()->to_string(), 
            session->get_address()->port(),
            session->get_address()->guid());
    }

    virtual void    session_closed(netpeer::peer_session_interface* session) override
    {
        printf("connection %s:%d, guid:%llu closed\n",
            session->get_address()->to_string(),
            session->get_address()->port(),
            session->get_address()->guid());
    }

    virtual void    session_msg_recv(netpeer::peer_session_interface* session, char* data, int32_t len) override
    {
        char str[1024] = { 0 };
        memcpy(str, data, len);

        printf("recv msg[%s] len[%d] from session guid[%lld] to address[%s:%d]\n",
            str, len,
            session->get_address()->guid(),
            session->get_address()->to_string(), session->get_address()->port());

        int32_t pre_bytes = netpeer::peer_factory::get_send_buffer_pre_bytes();
        char buffer[1024] = { 0 };
        memcpy(&buffer[pre_bytes], str, len);

        //session->send_msg(str, len, netpeer::priority_high, netpeer::reliable_ordered, 0);
        session->send_msg_with_prebuffer(&buffer[pre_bytes], len, netpeer::priority_high, netpeer::reliable_ordered, 0);
    }

    /** 一些其他事件 */
    virtual void    log_msg(netpeer::peer_log_level log_level, char* data, int32_t len) override
    {
        printf("[netpper_srv] [%s] %s\n", get_log_lvl_str(log_level), data);
    }
};

void server_test1(){
    std::cout << "input the port and max_connection:" << std::endl;
    int32_t port;
    int32_t max_connections;
    std::cin >> port >> max_connections;
    netpeer::peer_server_interface* server = netpeer::peer_factory::create_peer_server();
    server->set_event_handler(new server_event_handler());
    server->set_log_level(netpeer::peer_log_level_debug);
    if (!server->bind(max_connections, port))
    {
        printf("server bind failed.\n");
        return;
    }
    server->set_time_out(10000);

    while (true){
        if (!server->update(10)){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

int main()
{
    std::cout << "0, client test; 1, client test1, 2, server_test, 3, server_test1" << std::endl;
    int cmd = 0;
    std::cin >> cmd;
    if (cmd == 0){
        client_test();
    }
    else if (cmd == 1){
        client_test1();
    }
    else if (cmd == 2){
        server_test();
    }
    else if (cmd == 3){
        server_test1();
    }

    system("pause");
    return 0;
}