/*******************************************************************************
* 版权所有 (C)2009 深圳市迅雷网络技术有限公司。
* 系统名称  : 迅雷公共库
* 文件名称  : xl_log4cplus.h
* 内容摘要  : 要使用 xl_log4cplus 需要包含的唯一头文件
* 当前版本  : 1.0
* 作    者  : 杨晓虎
* 设计日期  : 2009年4月16日
* 修改记录  : 
* 日    期      版    本        修改人      修改摘要
*******************************************************************************/

/********************************* 使用说明 ***********************************/

/**************************** 条件编译选项和头文件 ****************************/
#ifndef __XL_LOG4CPLUS_H_77D80A29_3746_4E1D_AD42_855D11851904__
#define __XL_LOG4CPLUS_H_77D80A29_3746_4E1D_AD42_855D11851904__

#ifdef RECORD_LOG
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#endif

XL_NAMESPACE_BEGIN(lnx)

/****************************** XL_LOG4CPLUS 标准宏 ***************************/

#ifndef RECORD_LOG

#define LOG_INIT()
#define LOG_INIT_EX(cfgfile)

// log in class
#define LOG_CLS_DEC()
#define LOG_CLS_DEC_EX(loggername)
#define LOG_METHOD()
#define LOG_TRACE(msg)
#define LOG_DEBUG(msg)
#define LOG_INFO(msg)
#define LOG_WARN(msg)
#define LOG_ERROR(msg)
#define LOG_FATAL(msg)

// log in global
#define LOGGER_DEC(logger)
#define LOGGER_IMP(logger, loggername)
#define LOGGER_IMP_EX(logger, loggername)
#define LOGGER_METHOD(logger)
#define LOGGER_TRACE(logger,msg)
#define LOGGER_DEBUG(logger,msg)
#define LOGGER_INFO(logger,msg)
#define LOGGER_WARN(logger,msg)
#define LOGGER_ERROR(logger,msg)
#define LOGGER_FATAL(logger,msg)

#else // !defined(RECORD_LOG)

#define LOG_INIT()                              log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("xl_log4cplus.cfg"))
#define LOG_INIT_EX(cfgfile)                    log4cplus::PropertyConfigurator::doConfigure(cfgfile)

// log in global
#define LOGGER_DEC(logger)                        extern log4cplus::Logger logger
#define LOGGER_IMP(logger, loggername)            LOGGER_IMP_EX(logger, LOG4CPLUS_C_STR_TO_TSTRING(__FILE__))
#define LOGGER_IMP_EX(logger, loggername)         log4cplus::Logger logger = log4cplus::Logger::getInstance(loggername)
#define LOGGER_METHOD(logger)                     LOG4CPLUS_TRACE_METHOD(logger, LOG4CPLUS_C_STR_TO_TSTRING(__PRETTY_FUNCTION__))
#define LOGGER_TRACE(logger,msg)                  LOG4CPLUS_TRACE(logger, '[' << LOG4CPLUS_C_STR_TO_TSTRING(__FILE__) << LOG4CPLUS_C_STR_TO_TSTRING(" - ") << LOG4CPLUS_C_STR_TO_TSTRING(__PRETTY_FUNCTION__) << LOG4CPLUS_C_STR_TO_TSTRING(" - ") << __LINE__ << ']' << msg)
#define LOGGER_DEBUG(logger,msg)                  LOG4CPLUS_DEBUG(logger, '[' << LOG4CPLUS_C_STR_TO_TSTRING(__FILE__) << LOG4CPLUS_C_STR_TO_TSTRING(" - ") << LOG4CPLUS_C_STR_TO_TSTRING(__PRETTY_FUNCTION__) << LOG4CPLUS_C_STR_TO_TSTRING(" - ") << __LINE__ << ']' << msg)
#define LOGGER_INFO(logger,msg)                   LOG4CPLUS_INFO(logger,  '[' << LOG4CPLUS_C_STR_TO_TSTRING(__FILE__) << LOG4CPLUS_C_STR_TO_TSTRING(" - ") << LOG4CPLUS_C_STR_TO_TSTRING(__PRETTY_FUNCTION__) << LOG4CPLUS_C_STR_TO_TSTRING(" - ") << __LINE__ << ']' << msg)
#define LOGGER_WARN(logger,msg)                   LOG4CPLUS_WARN(logger,  '[' << LOG4CPLUS_C_STR_TO_TSTRING(__FILE__) << LOG4CPLUS_C_STR_TO_TSTRING(" - ") << LOG4CPLUS_C_STR_TO_TSTRING(__PRETTY_FUNCTION__) << LOG4CPLUS_C_STR_TO_TSTRING(" - ") << __LINE__ << ']' << msg)
#define LOGGER_ERROR(logger,msg)                  LOG4CPLUS_ERROR(logger, '[' << LOG4CPLUS_C_STR_TO_TSTRING(__FILE__) << LOG4CPLUS_C_STR_TO_TSTRING(" - ") << LOG4CPLUS_C_STR_TO_TSTRING(__PRETTY_FUNCTION__) << LOG4CPLUS_C_STR_TO_TSTRING(" - ") << __LINE__ << ']' << msg)
#define LOGGER_FATAL(logger,msg)                  LOG4CPLUS_FATAL(logger, '[' << LOG4CPLUS_C_STR_TO_TSTRING(__FILE__) << LOG4CPLUS_C_STR_TO_TSTRING(" - ") << LOG4CPLUS_C_STR_TO_TSTRING(__PRETTY_FUNCTION__) << LOG4CPLUS_C_STR_TO_TSTRING(" - ") << __LINE__ << ']' << msg)

// log in class
#define LOG_CLS_DEC()                           LOG_CLS_DEC_EX(LOG4CPLUS_C_STR_TO_TSTRING(__FILE__))
#define LOG_CLS_DEC_EX(loggername)              static log4cplus::Logger & getLogger() { static log4cplus::Logger s_logger = log4cplus::Logger::getInstance(loggername); return s_logger; }

#define LOG_METHOD()                            LOGGER_METHOD(getLogger())
#define LOG_TRACE(msg)                          LOGGER_TRACE(getLogger(),msg)
#define LOG_DEBUG(msg)                          LOGGER_DEBUG(getLogger(),msg)
#define LOG_INFO(msg)                           LOGGER_INFO(getLogger(),msg)
#define LOG_WARN(msg)                           LOGGER_WARN(getLogger(),msg)
#define LOG_ERROR(msg)                          LOGGER_ERROR(getLogger(),msg)
#define LOG_FATAL(msg)                          LOGGER_FATAL(getLogger(),msg)

#endif // RECORD_LOG


/********************************** 兼容看看的 TSLog **********************************/

#ifndef RECORD_LOG

#define TSTRACE
#define TSDEBUG
#define TSINFO
#define TSWARN
#define TSERROR
#define TSFATAL

#else // !defined(RECORD_LOG)

#define TSTRACE     LOG_TRACE
#define TSDEBUG     LOG_DEBUG
#define TSINFO      LOG_INFO
#define TSWARN      LOG_WARN
#define TSERROR     LOG_ERROR
#define TSFATAL     LOG_FATAL

#endif // RECORD_LOG

XL_NAMESPACE_END(lnx)

#endif // end of __XL_LOG4CPLUS_H_77D80A29_3746_4E1D_AD42_855D11851904__
