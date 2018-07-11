/**
 *
 * peer session interface
 *
 * 连接的基本接口
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-03-30
 */

#ifndef __common_rudp_netpeer_peer_session_interface_h_
#define __common_rudp_netpeer_peer_session_interface_h_

#include "peer_config.h"
#include "peer_define.h"
#include <stdint.h>

namespace netpeer
{
class peer_address_interface;
class np_api peer_session_interface
{
public:
    virtual ~peer_session_interface() {}

public:
    virtual peer_address_interface* get_address() = 0;
    virtual void    set_custom_data(void* data) = 0;
    virtual void*   get_custom_data() = 0;

    /** 
     * return 0 on bad input. Otherwise a number that identifies this message. 
     * If \a reliability is a type that returns a receipt, on a later call to Receive() 
     * you will get ID_SND_RECEIPT_ACKED or ID_SND_RECEIPT_LOSS with bytes 1-4 inclusive containing this number
     */
    virtual uint32_t send_msg(const char *data, const int32_t length, 
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

    virtual void    close() = 0;

    /** 
     * @brief get the connection statistis
     */
    virtual bool    get_statistics(np_statistics* nps) = 0;
};

}

#endif