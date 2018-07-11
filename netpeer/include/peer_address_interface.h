/**
 *
 * neetpeer.h
 *
 * neet peer adress 接口
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-03-30
 */

#ifndef __common_rudp_netpeer_address_interface_h__
#define __common_rudp_netpeer_address_interface_h__

#include <stdint.h>
#include "peer_config.h"

namespace netpeer
{

class np_api peer_address_interface
{
public:
    virtual ~peer_address_interface(){}

public:
    virtual const char* to_string() = 0;
    virtual int32_t     port() = 0;
    virtual uint64_t    guid() = 0;
};

}

#endif