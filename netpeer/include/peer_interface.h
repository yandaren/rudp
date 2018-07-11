/**
 *
 * peer_connector_interface
 *
 * peer interface
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-03-30
 */

#ifndef __common_rudp_netpeer_peer_interface_h__
#define __common_rudp_netpeer_peer_interface_h__

#include "peer_config.h"
#include "peer_define.h"
#include <stdint.h>

namespace netpeer
{
class peer_event_handler;
class np_api peer_interface
{
protected:
    peer_event_handler*  event_handler_;
    peer_log_level       log_lvl_;
public:
    peer_interface();
    virtual ~peer_interface(){}

public:
    virtual void            set_time_out(int32_t milli) = 0;
    virtual void            stop() = 0;
    virtual bool            update(float delta_time) = 0;

public:
    void            set_event_handler(peer_event_handler* handler);
    peer_event_handler* get_event_handler();
    void            set_log_level(peer_log_level lvl);

protected:
    void                    log_msg(peer_log_level log_level, const char* format, ...);
};
}

#endif