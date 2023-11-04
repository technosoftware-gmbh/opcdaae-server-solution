/*
 * Copyright (c) 2011-2022 Technosoftware GmbH. All rights reserved
 * Web: https://technosoftware.com 
 * 
 * The source code in this file is covered under a dual-license scenario:
 *   - Owner of a purchased license: SCLA 1.0
 *   - GPL V3: everybody else
 *
 * SCLA license terms accompanied with this source code.
 * See https://technosoftware.com/license/Source_Code_License_Agreement.pdf
 *
 * GNU General Public License as published by the Free Software Foundation;
 * version 3 of the License are accompanied with this source code.
 * See https://technosoftware.com/license/GPLv3License.txt
 *
 * This source code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include <string>

#include <cstdio>
#include "string.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <vector>
#include <map>
#include <list>
#include <queue>
#include <deque>

/**
 * @typedef int LoggerId
 *
 * @brief   Defines an alias representing identifier for the logger.
 */

typedef int LoggerId;

/** @brief   Identifier for the invalid logger. */
const int LOGGER_INVALID_LOGGER_ID = -1;

/** @brief   Identifier for the main logger. */
const int LOGGER_MAIN_LOGGER_ID = 0;

//! the main logger name. DO NOT TOUCH
const char*const LOGGER_MAIN_LOGGER_KEY = "Main";

//! check VC VERSION. DO NOT TOUCH
//! format micro cannot support VC6 or VS2003, please use stream input log, like LOGI, LOGD, LOG_DEBUG, LOG_STREAM ...
#if _MSC_VER >= 1400 //MSVC >= VS2005
#define LOGGER_FORMAT_INPUT_ENABLE
#endif

#ifndef WIN32
#define LOGGER_FORMAT_INPUT_ENABLE
#endif

#define  OPCWSTOAS( wstr ) {USES_CONVERSION; LPSTR OPCastr = W2A( wstr );

typedef enum 
{
    ///< An enum constant representing the log level trace option
    LOG_LEVEL_TRACE = 0,
    ///< An enum constant representing the log level debug option
    LOG_LEVEL_DEBUG,
    ///< An enum constant representing the log level Information option
    LOG_LEVEL_INFO,
    ///< An enum constant representing the log level Warning option
    LOG_LEVEL_WARN,
    ///< An enum constant representing the log level error option
    LOG_LEVEL_ERROR,
    ///< An enum constant representing the log level alarm option
    LOG_LEVEL_ALARM,
    ///< An enum constant representing the log level fatal option
    LOG_LEVEL_FATAL,
	///< An enum constant representing the log level fatal option
	LOG_LEVEL_DISABLE,
} ENUM_LOG_LEVEL;

/** @brief   The logger maximum. */
const int LOGGER_LOGGER_MAX = 20;

/** @brief   Maximum size of the logger content. */
const int LOGGER_LOG_BUF_SIZE = 1024 * 8;

/** @brief   Depth of the logger log container. */
const int LOGGER_LOG_CONTAINER_DEPTH = 5;

/** @brief   Size of the logger log queue limit. */
const int LOGGER_LOG_QUEUE_LIMIT_SIZE = 10000;

/** @brief   The logger all synchronous output. */
const bool LOGGER_ALL_SYNCHRONOUS_OUTPUT = false;

/** @brief   The logger all debugoutput display. */
const bool LOGGER_ALL_DEBUGOUTPUT_DISPLAY = false;

/** @brief   Full pathname of the logger default file. */
const char* const LOGGER_DEFAULT_PATH = "./log/";

/** @brief   The logger default level. */
const int LOGGER_DEFAULT_LEVEL = LOG_LEVEL_INFO;

/** @brief   The logger default display. */
const bool LOGGER_DEFAULT_DISPLAY = true;

/** @brief   The logger default outfile. */
const bool LOGGER_DEFAULT_OUTFILE = true;

/** @brief   The logger default monthdir. */
const bool LOGGER_DEFAULT_MONTHDIR = false;

/** @brief   The logger default limitsize. */
const int LOGGER_DEFAULT_LIMITSIZE = 50;

/** @brief   The logger default showsuffix. */
const bool LOGGER_DEFAULT_SHOWSUFFIX = true;

/* support ANSI->OEM console conversion on Windows */
#undef LOGGER_OEM_CONSOLE

/** @brief   Number of logger force reserve files. */
const size_t LOGGER_FORCE_RESERVE_FILE_COUNT = 7;

/**
 * @struct  LogData
 *
 * @brief   The log data
 */

struct LogData
{
    LoggerId _id;                               // dest logger id
    int    _type;                               // type.
    int    _typeval;
    int    _level;                              // log level
    time_t _time;                               // create time
    unsigned int _precise;                      // create time 
    int _contentLen;
    char _content[LOGGER_LOG_BUF_SIZE];         // content
};


/**
 * @class   LogManager
 *
 * @brief   Manager for logs.
 */

class LogManager
{
public:
    LogManager() {};
    virtual ~LogManager() {};

    /**
     * @fn  static LogManager * LogManager::getInstance();
     *
     * @brief   Gets the instance.
     *
     * @return  Null if it fails, else the instance.
     */

    static LogManager * getInstance();

    /**
     * @fn  inline static LogManager & LogManager::getRef()
     *
     * @brief   Gets the reference.
     *
     * @return  The reference.
     */

    inline static LogManager & getRef() { return *getInstance(); }

    /**
     * @fn  inline static LogManager * LogManager::getPtr()
     *
     * @brief   Gets the pointer.
     *
     * @return  Null if it fails, else the pointer.
     */

    inline static LogManager * getPtr() { return getInstance(); }

    /**
     * @fn  virtual bool LogManager::config(const char * configPath) = 0;
     *
     * @brief   Config or overwrite configure 
     *          Needs to be called before LogManager::Start,, OR Do not call.
     *
     * @param   configPath  Full pathname of the configuration file.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool config(const char * configPath) = 0;
    virtual bool configFromString(const char * configContent) = 0;

    /**
     * @fn  virtual LoggerId LogManager::createLogger(const char* key) = 0;
     *
     * @brief   Create or overwrite logger. 
     *          Needs to be called before LogManager::Start, OR Do not call.
     *
     * @param   key The key.
     *
     * @return  The new logger.
     */

    virtual LoggerId createLogger(const char* key) = 0;

    /**
     * @fn  virtual bool LogManager::start() = 0;
     *
     * @brief   Start Log Thread. This method can only be called once by one process.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool start() = 0;

    /**
     * @fn  virtual bool LogManager::stop() = 0;
     *
     * @brief   Default the method will be calling at process exit auto. 
     *          Default no need to call and not recommended.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool stop() = 0;

    /**
     * @fn  virtual LoggerId LogManager::findLogger(const char* key) = 0;
     *
     * @brief   Searches for a logger. 
     *          Thread safe.
     *
     * @param   key The key.
     *
     * @return  The found logger.
     */

    virtual LoggerId findLogger(const char* key) = 0;

    /**
     * @fn  virtual bool LogManager::prePushLog(LoggerId id, int level) = 0;
     *
     * @brief   pre-check the log filter. if filter out return false.
     *          Thread safe.
     *
     * @param   id      The identifier.
     * @param   level   The level.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool prePushLog(LoggerId id, int level) = 0;

    /**
     * @fn  virtual bool LogManager::pushLog(LogData * pLog, const char * file = NULL, int line = 0) = 0;
     *
     * @brief   Push log.
     *          Thread safe.
     *
     * @param [in,out]  pLog    If non-null, the log.
     * @param           file    (Optional) The file.
     * @param           line    (Optional) The line.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool pushLog(LogData * pLog, const char * file = NULL, int line = 0) = 0;

    /**
     * @fn  virtual bool LogManager::enableLogger(LoggerId id, bool enable) = 0;
     *
     * @brief   Enables the logger.
     *          Thread safe.
     *
     * @param   id      The identifier.
     * @param   enable  True to enable, false to disable.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool enableLogger(LoggerId id, bool enable) = 0;  

    /**
     * @fn  virtual bool LogManager::setLoggerName(LoggerId id, const char * name) = 0;
     *
     * @brief   Sets logger name.
     *          Thread safe.
     *
     * @param   id      The identifier.
     * @param   name    The name.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool setLoggerName(LoggerId id, const char * name) = 0;

    /**
     * @fn  virtual bool LogManager::setLoggerPath(LoggerId id, const char * path) = 0;
     *
     * @brief   Sets logger path.
     *          Thread safe.
     *
     * @param   id      The identifier.
     * @param   path    Full pathname of the file.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool setLoggerPath(LoggerId id, const char * path) = 0;

    /**
     * @fn  virtual bool LogManager::setLoggerLevel(LoggerId id, int nLevel) = 0;
     *
     * @brief   Sets logger level.
     *          Thread safe.
     *
     * @param   id      The identifier.
     * @param   nLevel  The level.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool setLoggerLevel(LoggerId id, int nLevel) = 0;  

    /**
     * @fn  virtual bool LogManager::setLoggerFileLine(LoggerId id, bool enable) = 0;
     *
     * @brief   Sets logger file line.
     *          Thread safe.
     *
     * @param   id      The identifier.
     * @param   enable  True to enable, false to disable.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool setLoggerFileLine(LoggerId id, bool enable) = 0;

    /**
     * @fn  virtual bool LogManager::setLoggerDisplay(LoggerId id, bool enable) = 0;
     *
     * @brief   Sets logger display.
     *          Thread safe.
     *
     * @param   id      The identifier.
     * @param   enable  True to enable, false to disable.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool setLoggerDisplay(LoggerId id, bool enable) = 0;

    /**
     * @fn  virtual bool LogManager::setLoggerOutFile(LoggerId id, bool enable) = 0;
     *
     * @brief   Sets logger out file.
     *          Thread safe.
     *
     * @param   id      The identifier.
     * @param   enable  True to enable, false to disable.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool setLoggerOutFile(LoggerId id, bool enable) = 0;

    /**
     * @fn  virtual bool LogManager::setLoggerLimitsize(LoggerId id, unsigned int limitsize) = 0;
     *
     * @brief   Sets logger limitsize.
     *          Thread safe.
     *
     * @param   id          The identifier.
     * @param   limitsize   The limitsize.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool setLoggerLimitsize(LoggerId id, unsigned int limitsize) = 0;

    /**
     * @fn  virtual bool LogManager::setLoggerMonthdir(LoggerId id, bool enable) = 0;
     *
     * @brief   Sets logger monthdir.
     *          Thread safe.
     *
     * @param   id      The identifier.
     * @param   enable  True to enable, false to disable.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool setLoggerMonthdir(LoggerId id, bool enable) = 0;

    /**
     * @fn  virtual bool LogManager::setLoggerReserveTime(LoggerId id, time_t sec) = 0;
     *
     * @brief   Sets logger reserve time.
     *          Thread safe.
     *
     * @param   id  The identifier.
     * @param   sec The security.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool setLoggerReserveTime(LoggerId id, time_t sec) = 0;


    /**
     * @fn  virtual bool LogManager::setAutoUpdate(int interval) = 0;
     *
     * @brief   Sets automatic update.
     *          Thread safe.
     *
     * @param   interval    The interval.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool setAutoUpdate(int interval/*per second, 0 is disable auto update*/) = 0;

    /**
     * @fn  virtual bool LogManager::updateConfig() = 0;
     *
     * @brief   Updates the configuration.
     *
     * @return  True if it succeeds, false if it fails.
     */

    virtual bool updateConfig() = 0;

    //! Logger status statistics, thread safe.
    virtual bool isLoggerEnable(LoggerId id) = 0;
    virtual unsigned long long getStatusTotalWriteCount() = 0;
    virtual unsigned long long getStatusTotalWriteBytes() = 0;
    virtual unsigned long long getStatusTotalPushQueue() = 0;
    virtual unsigned long long getStatusTotalPopQueue() = 0;
    virtual unsigned int getStatusActiveLoggers() = 0;

    virtual LogData * makeLogData(LoggerId id, int level) = 0;
    virtual void freeLogData(LogData * log) = 0;
};

class LoggerStream;
class LoggerBinary;

//! base macro.
#define LOG_STREAM(id, level, file, line, log)\
do{\
    if (LogManager::getPtr()->prePushLog(id,level)) \
    {\
        LogData * __pLog = LogManager::getPtr()->makeLogData(id, level); \
        LoggerStream __ss(__pLog->_content + __pLog->_contentLen, LOGGER_LOG_BUF_SIZE - __pLog->_contentLen);\
        __ss << log;\
        __pLog->_contentLen += __ss.getCurrentLen(); \
		LogManager::getPtr()->pushLog(__pLog, file, line);\
    }\
} while (0)


//! fast macro
#define LOG_TRACE(id, log) LOG_STREAM(id, LOG_LEVEL_TRACE, __FILE__, __LINE__, log)
#define LOG_DEBUG(id, log) LOG_STREAM(id, LOG_LEVEL_DEBUG, __FILE__, __LINE__, log)
#define LOG_INFO(id, log)  LOG_STREAM(id, LOG_LEVEL_INFO, __FILE__, __LINE__, log)
#define LOG_WARN(id, log)  LOG_STREAM(id, LOG_LEVEL_WARN, __FILE__, __LINE__, log)
#define LOG_ERROR(id, log) LOG_STREAM(id, LOG_LEVEL_ERROR, __FILE__, __LINE__, log)
#define LOG_ALARM(id, log) LOG_STREAM(id, LOG_LEVEL_ALARM, __FILE__, __LINE__, log)
#define LOG_FATAL(id, log) LOG_STREAM(id, LOG_LEVEL_FATAL, __FILE__, __LINE__, log)

//! super macro.
#define LOGT( log ) LOG_TRACE(LOGGER_MAIN_LOGGER_ID, log )
#define LOGD( log ) LOG_DEBUG(LOGGER_MAIN_LOGGER_ID, log )
#define LOGI( log ) LOG_INFO(LOGGER_MAIN_LOGGER_ID, log )
#define LOGW( log ) LOG_WARN(LOGGER_MAIN_LOGGER_ID, log )
#define LOGE( log ) LOG_ERROR(LOGGER_MAIN_LOGGER_ID, log )
#define LOGA( log ) LOG_ALARM(LOGGER_MAIN_LOGGER_ID, log )
#define LOGF( log ) LOG_FATAL(LOGGER_MAIN_LOGGER_ID, log )


//! format input log.
#ifdef LOGGER_FORMAT_INPUT_ENABLE
#ifdef WIN32
#define LOG_FORMAT(id, level, file, line, logformat, ...) \
do{ \
    if (LogManager::getPtr()->prePushLog(id,level)) \
    {\
        LogData * __pLog = LogManager::getPtr()->makeLogData(id, level); \
        int __logLen = _snprintf_s(__pLog->_content + __pLog->_contentLen, LOGGER_LOG_BUF_SIZE - __pLog->_contentLen, _TRUNCATE, logformat, ##__VA_ARGS__); \
        if (__logLen < 0) __logLen = LOGGER_LOG_BUF_SIZE - __pLog->_contentLen; \
        __pLog->_contentLen += __logLen; \
		LogManager::getPtr()->pushLog(__pLog, file, line);\
    }\
} while (0)
#else
#define LOG_FORMAT(id, level, file, line, logformat, ...) \
do{ \
    if (LogManager::getPtr()->prePushLog(id,level)) \
    {\
        LogData * __pLog = LogManager::getPtr()->makeLogData(id, level); \
        int __logLen = snprintf(__pLog->_content + __pLog->_contentLen, LOGGER_LOG_BUF_SIZE - __pLog->_contentLen,logformat, ##__VA_ARGS__); \
        if (__logLen < 0) __logLen = 0; \
        if (__logLen > LOGGER_LOG_BUF_SIZE - __pLog->_contentLen) __logLen = LOGGER_LOG_BUF_SIZE - __pLog->_contentLen; \
        __pLog->_contentLen += __logLen; \
        LogManager::getPtr()->pushLog(__pLog, file, line); \
    } \
}while(0)
#endif
//!format string
#define LOGFMT_TRACE(id, fmt, ...)  LOG_FORMAT(id, LOG_LEVEL_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGFMT_DEBUG(id, fmt, ...)  LOG_FORMAT(id, LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGFMT_INFO(id, fmt, ...)  LOG_FORMAT(id, LOG_LEVEL_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGFMT_WARN(id, fmt, ...)  LOG_FORMAT(id, LOG_LEVEL_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGFMT_ERROR(id, fmt, ...)  LOG_FORMAT(id, LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGFMT_ALARM(id, fmt, ...)  LOG_FORMAT(id, LOG_LEVEL_ALARM, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGFMT_FATAL(id, fmt, ...)  LOG_FORMAT(id, LOG_LEVEL_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGFMTT( fmt, ...) LOGFMT_TRACE(LOGGER_MAIN_LOGGER_ID, fmt,  ##__VA_ARGS__)
#define LOGFMTD( fmt, ...) LOGFMT_DEBUG(LOGGER_MAIN_LOGGER_ID, fmt,  ##__VA_ARGS__)
#define LOGFMTI( fmt, ...) LOGFMT_INFO(LOGGER_MAIN_LOGGER_ID, fmt,  ##__VA_ARGS__)
#define LOGFMTW( fmt, ...) LOGFMT_WARN(LOGGER_MAIN_LOGGER_ID, fmt,  ##__VA_ARGS__)
#define LOGFMTE( fmt, ...) LOGFMT_ERROR(LOGGER_MAIN_LOGGER_ID, fmt,  ##__VA_ARGS__)
#define LOGFMTA( fmt, ...) LOGFMT_ALARM(LOGGER_MAIN_LOGGER_ID, fmt,  ##__VA_ARGS__)
#define LOGFMTF( fmt, ...) LOGFMT_FATAL(LOGGER_MAIN_LOGGER_ID, fmt,  ##__VA_ARGS__)
#else
inline void empty_log_format_function1(LoggerId id, const char*, ...) {}
inline void empty_log_format_function2(const char*, ...) {}
#define LOGFMT_TRACE empty_log_format_function1
#define LOGFMT_DEBUG LOGFMT_TRACE
#define LOGFMT_INFO LOGFMT_TRACE
#define LOGFMT_WARN LOGFMT_TRACE
#define LOGFMT_ERROR LOGFMT_TRACE
#define LOGFMT_ALARM LOGFMT_TRACE
#define LOGFMT_FATAL LOGFMT_TRACE
#define LOGFMTT empty_log_format_function2
#define LOGFMTD LOGFMTT
#define LOGFMTI LOGFMTT
#define LOGFMTW LOGFMTT
#define LOGFMTE LOGFMTT
#define LOGFMTA LOGFMTT
#define LOGFMTF LOGFMTT
#endif

//! optimze from std::stringstream to LoggerStream
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4996)
#endif
class LoggerBinary
{
public:
    LoggerBinary(const void * buf, int len)
    {
        _buf = static_cast<const char*>(buf);
        _len = len;
    }
    const char * _buf;
    int  _len;
};
class LoggerStream
{
public:
    inline LoggerStream(char * buf, int len);
    inline int getCurrentLen() { return (int)(cur_ - begin_); }
private:
    template<class T>
    inline LoggerStream & writeData(const char * ft, T t);
    inline LoggerStream & writeLongLong(long long t);
    inline LoggerStream & writeULongLong(unsigned long long t);
    inline LoggerStream & writePointer(const void * t);
    inline LoggerStream & writeString(const char * t, size_t len);
    inline LoggerStream & writeWString(const wchar_t* t);
    inline LoggerStream & writeBinary(const LoggerBinary & t);
public:
    inline LoggerStream & operator <<(const void * t) { return  writePointer(t); }

    inline LoggerStream & operator <<(const char * t) { return writeString(t, strlen(t)); }
#ifdef WIN32
    inline LoggerStream & operator <<(const wchar_t * t) { return writeWString(t); }
#endif
    inline LoggerStream & operator <<(bool t) { return (t ? writeData("%s", "true") : writeData("%s", "false")); }

    inline LoggerStream & operator <<(char t) { return writeData("%c", t); }

    inline LoggerStream & operator <<(unsigned char t) { return writeData("%u", (unsigned int)t); }

    inline LoggerStream & operator <<(short t) { return writeData("%d", (int)t); }

    inline LoggerStream & operator <<(unsigned short t) { return writeData("%u", (unsigned int)t); }

    inline LoggerStream & operator <<(int t) { return writeData("%d", t); }

    inline LoggerStream & operator <<(unsigned int t) { return writeData("%u", t); }

    inline LoggerStream & operator <<(long t) { return writeLongLong(t); }

    inline LoggerStream & operator <<(unsigned long t) { return writeULongLong(t); }

    inline LoggerStream & operator <<(long long t) { return writeLongLong(t); }

    inline LoggerStream & operator <<(unsigned long long t) { return writeULongLong(t); }

    inline LoggerStream & operator <<(float t) { return writeData("%.4f", t); }

    inline LoggerStream & operator <<(double t) { return writeData("%.4lf", t); }

    template<class _Elem, class _Traits, class _Alloc> //support std::string, std::wstring
    inline LoggerStream & operator <<(const std::basic_string<_Elem, _Traits, _Alloc> & t) { return writeString(t.c_str(), t.length()); }

    inline LoggerStream & operator << (const LoggerBinary & binary) { return writeBinary(binary); }

    template<class _Ty1, class _Ty2>
    inline LoggerStream & operator <<(const std::pair<_Ty1, _Ty2> & t) { return *this << "pair(" << t.first << ":" << t.second << ")"; }

    template<class _Elem, class _Alloc>
    inline LoggerStream & operator <<(const std::vector<_Elem, _Alloc> & t)
    {
        *this << "vector(" << t.size() << ")[";
        int inputCount = 0;
        for (typename std::vector<_Elem, _Alloc>::const_iterator iter = t.begin(); iter != t.end(); iter++)
        {
            inputCount++;
            if (inputCount > LOGGER_LOG_CONTAINER_DEPTH)
            {
                *this << "..., ";
                break;
            }
            *this << *iter << ", ";
        }
        if (!t.empty())
        {
            cur_ -= 2;
        }
        return *this << "]";
    }
    template<class _Elem, class _Alloc>
    inline LoggerStream & operator <<(const std::list<_Elem, _Alloc> & t)
    {
        *this << "list(" << t.size() << ")[";
        int inputCount = 0;
        for (typename std::list<_Elem, _Alloc>::const_iterator iter = t.begin(); iter != t.end(); iter++)
        {
            inputCount++;
            if (inputCount > LOGGER_LOG_CONTAINER_DEPTH)
            {
                *this << "..., ";
                break;
            }
            *this << *iter << ", ";
        }
        if (!t.empty())
        {
            cur_ -= 2;
        }
        return *this << "]";
    }
    template<class _Elem, class _Alloc>
    inline LoggerStream & operator <<(const std::deque<_Elem, _Alloc> & t)
    {
        *this << "deque(" << t.size() << ")[";
        int inputCount = 0;
        for (typename std::deque<_Elem, _Alloc>::const_iterator iter = t.begin(); iter != t.end(); iter++)
        {
            inputCount++;
            if (inputCount > LOGGER_LOG_CONTAINER_DEPTH)
            {
                *this << "..., ";
                break;
            }
            *this << *iter << ", ";
        }
        if (!t.empty())
        {
            cur_ -= 2;
        }
        return *this << "]";
    }
    template<class _Elem, class _Alloc>
    inline LoggerStream & operator <<(const std::queue<_Elem, _Alloc> & t)
    {
        *this << "queue(" << t.size() << ")[";
        int inputCount = 0;
        for (typename std::queue<_Elem, _Alloc>::const_iterator iter = t.begin(); iter != t.end(); iter++)
        {
            inputCount++;
            if (inputCount > LOGGER_LOG_CONTAINER_DEPTH)
            {
                *this << "..., ";
                break;
            }
            *this << *iter << ", ";
        }
        if (!t.empty())
        {
            cur_ -= 2;
        }
        return *this << "]";
    }
    template<class _K, class _V, class _Pr, class _Alloc>
    inline LoggerStream & operator <<(const std::map<_K, _V, _Pr, _Alloc> & t)
    {
        *this << "map(" << t.size() << ")[";
        int inputCount = 0;
        for (typename std::map < _K, _V, _Pr, _Alloc>::const_iterator iter = t.begin(); iter != t.end(); iter++)
        {
            inputCount++;
            if (inputCount > LOGGER_LOG_CONTAINER_DEPTH)
            {
                *this << "..., ";
                break;
            }
            *this << *iter << ", ";
        }
        if (!t.empty())
        {
            cur_ -= 2;
        }
        return *this << "]";
    }

private:
    LoggerStream() = default;
    LoggerStream(LoggerStream &) {}
    char* begin_ = nullptr;
    char*  end_ = nullptr;
    char*  cur_ = nullptr;
};

inline LoggerStream::LoggerStream(char * buf, int len)
{
    begin_ = buf;
    end_ = buf + len;
    cur_ = begin_;
}

template<class T>
inline LoggerStream& LoggerStream::writeData(const char * ft, T t)
{
    if (cur_ < end_)
    {
        int len = 0;
        int count = (int)(end_ - cur_);
#ifdef WIN32
        len = _snprintf(cur_, count, ft, t);
        if (len == count || len < 0)
        {
            len = count;
            *(end_ - 1) = '\0';
        }
#else
        len = snprintf(_cur, count, ft, t);
        if (len < 0)
        {
            *_cur = '\0';
            len = 0;
        }
        else if (len >= count)
        {
            len = count;
            *(_end - 1) = '\0';
        }
#endif
        cur_ += len;
    }
    return *this;
}

inline LoggerStream & LoggerStream::writeLongLong(long long t)
{
#ifdef WIN32  
    writeData("%I64d", t);
#else
    writeData("%lld", t);
#endif
    return *this;
}

inline LoggerStream & LoggerStream::writeULongLong(unsigned long long t)
{
#ifdef WIN32  
    writeData("%I64u", t);
#else
    writeData("%llu", t);
#endif
    return *this;
}

inline LoggerStream & LoggerStream::writePointer(const void * t)
{
#ifdef WIN32
    sizeof(t) == 8 ? writeData("%016I64x", (unsigned long long)t) : writeData("%08I64x", (unsigned long long)t);
#else
    sizeof(t) == 8 ? writeData("%016llx", (unsigned long long)t) : writeData("%08llx", (unsigned long long)t);
#endif
    return *this;
}

inline LoggerStream & LoggerStream::writeBinary(const LoggerBinary & t)
{
    writeData("%s", "\r\n\t[");
    for (int i = 0; i < (t._len / 16) + 1; i++)
    {
        writeData("%s", "\r\n\t");
        *this << (void*)(t._buf + i * 16);
        writeData("%s", ": ");
        for (int j = i * 16; j < (i + 1) * 16 && j < t._len; j++)
        {
            writeData("%02x ", (unsigned char)t._buf[j]);
        }
        writeData("%s", "\r\n\t");
        *this << (void*)(t._buf + i * 16);
        writeData("%s", ": ");
        for (int j = i * 16; j < (i + 1) * 16 && j < t._len; j++)
        {
            if (isprint((unsigned char)t._buf[j]))
            {
                writeData(" %c ", t._buf[j]);
            }
            else
            {
                *this << " . ";
            }
        }
    }

    writeData("%s", "\r\n\t]\r\n\t");
    return *this;
}
inline LoggerStream & LoggerStream::writeString(const char * t, size_t len)
{
    if (cur_ < end_)
    {
        size_t count = (size_t)(end_ - cur_);
        if (len > count)
        {
            len = count;
        }
        memcpy(cur_, t, len);
        cur_ += len;
        if (cur_ >= end_ - 1)
        {
            *(end_ - 1) = '\0';
        }
        else
        {
            *(cur_ + 1) = '\0';
        }
    }
    return *this;
}
inline LoggerStream & LoggerStream::writeWString(const wchar_t* t)
{
#ifdef WIN32
    DWORD dwLen = WideCharToMultiByte(CP_ACP, 0, t, -1, NULL, 0, NULL, NULL);
    if (dwLen < LOGGER_LOG_BUF_SIZE)
    {
        std::string str;
        str.resize(dwLen, '\0');
        dwLen = WideCharToMultiByte(CP_ACP, 0, t, -1, &str[0], dwLen, NULL, NULL);
        if (dwLen > 0)
        {
            writeData("%s", str.c_str());
        }
    }
#else
    //not support
#endif
    return *this;
}

#ifdef WIN32
#pragma warning(pop)
#endif
#endif
