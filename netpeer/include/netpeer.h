/**
 *
 * neetpeer.h
 * 
 * neet peer 接口
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-03-29
 */

#ifndef __common_rudp_netpeer_h__
#define __common_rudp_netpeer_h__

#include "peer_event_handler.h"
#include "peer_address_interface.h"
#include "peer_connector_interface.h"
#include "peer_server_interface.h"
#include "peer_session_interface.h"

namespace netpeer
{

/** 
 * Notice: the peer_connector_interface and peer_server_interface both not thread safe
 */

class np_api peer_factory
{
public:
    /** create the connector */

    /** 
     * @brief
     */
    static  peer_connector_interface* create_peer_connector();

    /** 
     * @brief destroy the connector
     */
    static  void                      destory_peer_connector(peer_connector_interface* p);

    /** 
     * create a peer server instance 
     */
    static  peer_server_interface*    create_peer_server();

    /** 
     * @brief destroy the server
     */
    static  void                      destory_peer_server(peer_server_interface* p);

    /** get send buffer pre buffer byte */
    static  uint32_t                  get_send_buffer_pre_bytes();

    /** get peer time **/
    static  uint64_t                  get_time_in_nanosecond();
};

}
#endif