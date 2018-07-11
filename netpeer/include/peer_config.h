/**
 *
 * config
 *
 * 不同系统下的一些宏定义
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-03-29
 */

#ifndef __common_rudp_netpeer_config_h__
#define __common_rudp_netpeer_config_h__

#if defined(_WIN32)
    #define np_import   __declspec(dllimport)
    #define np_export   __declspec(dllexport)
#else
    #define np_import   __attribute__((visibility("default")))
    #define np_export   __attribute__((visibility("default")))
#endif

#ifndef NP_STATIC
    #ifdef NP_DLL
        #define np_api  np_export
    #else
        #define np_api  np_import
    #endif
#endif

#ifndef np_api
    #define np_api
#endif

#endif