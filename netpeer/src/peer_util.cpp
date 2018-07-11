#include "peer_util.h"
#include "MessageIdentifiers.h"
#include "RakNetDefines.h"
#include "RakNetTypes.h"

namespace netpeer{

    uint32_t peer_util::get_indentifier_offset(RakNet::Packet* p)
    {
        if ((unsigned char)p->data[0] == ID_TIMESTAMP)
        {
            return sizeof(RakNet::MessageID) + sizeof(RakNet::Time);
        }
        else
            return 0;
    }

    uint8_t  peer_util::get_packet_identifier(RakNet::Packet* p)
    {
        if (p == 0)
            return 255;

        return (uint8_t)p->data[get_indentifier_offset(p)];
    }

    uint32_t peer_util::get_userdata_offset(RakNet::Packet* p)
    {
        return get_indentifier_offset(p) + sizeof(RakNet::MessageID);
    }

    const char*  peer_util::get_peer_start_error_by_code(int32_t error_code)
    {
        static const char* error_str[] = {
            "raknet_started",
            "raknet_already_started",
            "invalid_socket_descriptors",
            "invalid_max_connects",
            "socket_family_not_supported",
            "socket_port_already_in_use",
            "socket_failed_to_bind",
            "socket_failed_test_send",
            "port_cannot_be_zero",
            "failed_to_create_network_thread",
            "could_not_generate_guid",
            "startup_other_failed",
        };

        if (error_code >= RakNet::RAKNET_STARTED &&
            error_code <= RakNet::STARTUP_OTHER_FAILURE)
        {
            return error_str[error_code];
        }

        static const char* unknow_error = "unknow_error";
        return unknow_error;
    }

    const char*  peer_util::get_peer_connect_error_by_code(int32_t error_code)
    {
        static const char* error_str[] = {
            "connection_attempt_started",
            "invalid_parameter",
            "cannot_resolve_domain_name",
            "already_connected_to_endpoint",
            "connection_attemp_already_in_progress",
            "security_initialization_failed",
        };

        if (error_code >= RakNet::CONNECTION_ATTEMPT_STARTED &&
            error_code <= RakNet::SECURITY_INITIALIZATION_FAILED)
        {
            return error_str[error_code];
        }

        static const char* unknow_error = "unknow_error";
        return unknow_error;
    }
}