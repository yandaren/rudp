/**
 *
 * event handler
 *
 * 一些事件定义
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-03-30
 */

#ifndef __common_rudp_netpeer_event_handler_h__
#define __common_rudp_netpeer_event_handler_h__

#include <stdint.h>
#include "peer_config.h"
#include "peer_define.h"

namespace netpeer
{
class peer_session_interface;
class np_api peer_event_handler
{
public:
    virtual ~peer_event_handler() {}

public:
    
    /** 一些连接事件 */
    virtual void    session_opened(peer_session_interface* session) = 0;

    /** 
     * @brief notice
     * while the application layer receive the session closed event, then should disslove the reference of the session, 
     * case it will freed in the next later
     */
    virtual void    session_closed(peer_session_interface* session) = 0;
    virtual void    session_msg_recv(peer_session_interface* session, char* data, int32_t len) = 0;

    /** 一些其他事件 */
    virtual void    log_msg(peer_log_level log_level, char* data, int32_t len) = 0;
};
}

#endif