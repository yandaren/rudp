#include "netpeer.h"
#include <iostream>
#include <thread>
#include <string>

static const char* get_log_lvl_str(netpeer::peer_log_level lvl){
    static const char* log_level_strs[] = { "debug", "info", "warn", "error", "fatal" };
    static const char* uknow_level = "unknow";
    if (lvl >= netpeer::peer_log_level_debug &&
        lvl <= netpeer::peer_log_level_fatal){
        return log_level_strs[lvl];
    }
    return uknow_level;
}

static std::string np_statistics_to_string(const netpeer::np_statistics& nps){
    char buffer[1024];
    sprintf(buffer,
        "Actual bytes per second sent         %llu\n"
        "Actual bytes per second received     %llu\n"
        "Message bytes per second sent        %llu\n"
        "Message bytes per second resent      %llu\n"
        "Message bytes per second pushed      %llu\n"
        "Message bytes per second returned    %llu\n"
        "Message bytes per second ignored     %llu\n"
        "Total bytes sent                     %llu\n"
        "Total bytes received                 %llu\n"
        "Total message bytes sent             %llu\n"
        "Total message bytes resent           %llu\n"
        "Total message bytes pushed           %llu\n"
        "Total message bytes returned         %llu\n"
        "Total message bytes ignored          %llu\n"
        "Elapsed connection time in seconds   %llu\n",
        nps.last_second[netpeer::actual_bytes_sent],
        nps.last_second[netpeer::actual_bytes_received],
        nps.last_second[netpeer::user_message_bytes_sent],
        nps.last_second[netpeer::user_message_bytes_resent],
        nps.last_second[netpeer::user_message_bytes_pushed],
        nps.last_second[netpeer::user_message_bytes_received_processed],
        nps.last_second[netpeer::user_message_bytes_received_ignored],
        nps.running_total[netpeer::actual_bytes_sent],
        nps.running_total[netpeer::actual_bytes_received],
        nps.running_total[netpeer::user_message_bytes_sent],
        nps.running_total[netpeer::user_message_bytes_resent],
        nps.running_total[netpeer::user_message_bytes_pushed],
        nps.running_total[netpeer::user_message_bytes_received_processed],
        nps.running_total[netpeer::user_message_bytes_received_ignored],
        (netpeer::peer_factory::get_time_in_nanosecond() - nps.connection_start_time) / 1000000);

    return buffer;
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

        static int32_t count = 0;
        count++;
        if (count % 2 == 0){
            netpeer::np_statistics nps;
            session->get_statistics(&nps);
            std::string nps_str = np_statistics_to_string(nps);
            printf("nps: \n%s\n", nps_str.c_str());
        }
    }

    /** 一些其他事件 */
    virtual void    log_msg(netpeer::peer_log_level log_level, char* data, int32_t len) override
    {
        printf("[netpper] [%s] %s.\n", get_log_lvl_str(log_level), data);
    }
};

void client_test(){

    std::cout << "input the remote ip and port:" << std::endl;
    std::string ip;
    int port;
    std::cin >> ip >> port;

    netpeer::peer_connector_interface* connector = netpeer::peer_factory::create_peer_connector();
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

		if (!connector->is_connected()) {
			printf("the connection not connected\n");
			continue;
		}

        char send_data[1024] = { 0 };
        memcpy(&send_data[pre_bytes], msg.c_str(), msg.size());
        connector->send_msg_with_prebuffer(&send_data[pre_bytes], (int32_t)msg.size(), netpeer::priority_high, netpeer::reliable_ordered, 0);
        //connector->send_msg(msg.c_str(), msg.size(), netpeer::priority_high, netpeer::reliable_ordered, 0);

        count++;
        if (count % 5 == 0){
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

        static int32_t count = 0;
        count++;
        if (count % 3 == 0){
            netpeer::np_statistics nps;
            session->get_statistics(&nps);
            std::string nps_str = np_statistics_to_string(nps);
            printf("nps: \n%s\n", nps_str.c_str());
        }
    }

    /** 一些其他事件 */
    virtual void    log_msg(netpeer::peer_log_level log_level, char* data, int32_t len) override
    {
        printf("[netpper_srv] [%s] %s\n", get_log_lvl_str(log_level), data);
    }
};

void server_test(){
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
    std::cout << "0, client test; 1, server_test" << std::endl;
    int cmd = 0;
    std::cin >> cmd;
    if (cmd == 0){
        client_test();
    }
    else if (cmd == 1){
        server_test();
    }

    system("pause");
    return 0;
}