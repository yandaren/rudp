/**
 *
 * peer_connector_interface
 *
 * peer client interface
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-03-30
 */

#ifndef __common_rudp_netpeer_connector_interface_h__
#define __common_rudp_netpeer_connector_interface_h__

#include "peer_interface.h"
#include "peer_define.h"

namespace netpeer
{

class np_api peer_connector_interface : public peer_interface
{
public:
    virtual ~peer_connector_interface(){}

public:
    virtual  bool   connect(const char* ip, int32_t port) = 0;
    virtual  bool   is_connected() = 0;
    virtual  void   disconnect() = 0;
    virtual  uint32_t send_msg(const char *data, const int32_t length,
                        packet_priority priority, packet_reliability reliability, char ordering_channel) = 0;

   /*
    * notice here, if use this api, you need give free byte prevent the data,
    * you can get the pre size from the api peer_factory::get_send_buffer_pre_bytes(),
    * this api will be faster then the api 'send_msg' case it will use the given buffer to package protocol rather then
    * allocate a new buffer to package packet, it has one less operatation of memory copy that api 'send_msg'
    * you may the use the api like this:
    *
    *  char buffer[1024];
    *  int32_t pre_bytes = peer_factory::get_send_buffer_pre_bytes();
    *  memcpy(&buffer[pre_bytes], msg, msg_len);
    *  peer->send_msg_with_prebuffer(&buffer[pre_bytes], length, priority, reliability, ordering_channel);
    *
    * notice!!!, if you have't given the pre bytes, it will crash
    */
    virtual uint32_t send_msg_with_prebuffer(const char *data, const int32_t length,
        packet_priority priority, packet_reliability reliability, char ordering_channel) = 0;
};
}

#endif