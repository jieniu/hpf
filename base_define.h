/*
 * filename      : base_define.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-17 16:31
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _BASE_DEFINE_H_
#define _BASE_DEFINE_H_
#include <stdio.h>
#include "global_var.h"

namespace hpf
{

enum ConnectionStatus
{
    NORMAL_STATUS  = 0,
    CLOSING_STATUS = 1,
    WAITING_STATUS = 2,
    WRITING_STATUS = 3,
    READING_STATUS = 4,
};


#define ret_if_fail(condition) \
    if (!(condition)) \
    {\
        LOGGER_ERROR(g_framework_logger, #condition" failed at " << __FILE__ << ":" << __LINE__);\
        return;\
    } 

#define ret_val_if_fail(condition, value)\
    if (!(condition)) \
    {\
        LOGGER_ERROR(g_framework_logger, #condition" failed at " << __FILE__ << ":" << __LINE__);\
        return value;\
    }

}
#endif
