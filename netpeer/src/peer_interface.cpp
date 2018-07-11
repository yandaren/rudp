#include "peer_interface.h"
#include "peer_event_handler.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif

namespace netpeer
{
    static const char* get_loglvl_str(peer_log_level lvl){
        static const char* log_level_strs[] = { "debug", "info", "warn", "error", "fatal" };
        static const char* uknow_level = "unknow";
        if (lvl >= peer_log_level_debug &&
            lvl <= peer_log_level_fatal){
            return log_level_strs[lvl];
        }
        return uknow_level;
    }

    static void defualt_log_handler(peer_log_level log_level, const char* data, int32_t len)
    {
        printf("[%s] %s\n", get_loglvl_str(log_level), data);
    }

    peer_interface::peer_interface()
        : event_handler_(nullptr)
        , log_lvl_(peer_log_level_debug)
    {

    }

    void            peer_interface::set_event_handler(peer_event_handler* handler)
    {
        event_handler_ = handler;
    }

    peer_event_handler* peer_interface::get_event_handler()
    {
        return event_handler_;
    }

    void            peer_interface::set_log_level(peer_log_level lvl)
    {
        log_lvl_ = lvl;
    }

    void   peer_interface::log_msg(peer_log_level log_level, const char* format, ...)
    {
        if (log_level < log_lvl_)
            return;

#define max_log_len 1023
        char buffer[max_log_len + 1];
        char* log_buff = buffer;
        va_list ap;
        va_start(ap, format);
        int32_t len = vsnprintf(0, 0, format, ap);
		va_end(ap);

        if (len > max_log_len){
            log_buff = (char*)malloc(len + 1);
        }
		va_start(ap, format);
        len = vsprintf(log_buff, format, ap);
        va_end(ap);

        if (event_handler_){
            event_handler_->log_msg(log_level, log_buff, len);
        }
        else{
            defualt_log_handler(log_level, log_buff, len);
        }

        if (log_buff != buffer){
            free(log_buff);
        }
#undef max_log_len
    }
}