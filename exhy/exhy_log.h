#ifndef __EXHY_LOG_H__
#define __EXHY_LOG_H__

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include <map>
#include <functional>
#include <stdarg.h>
#include <stdlib.h>
#include "exhy_thread.h"
#include "exhy_util.h"
#include "exhy_singleton.h" 
//#define _GLIBCXX_USE_CXX11_ABI 0

#define EXHY_LOG_LEVEL(logger,level) \
if( logger->getLevel() <= level) \
    exhy::LogEventWrap(exhy::LogEvent::ptr(new exhy::LogEvent(logger,level,__FILE__,__LINE__,0,exhy::GetThreadId(),\
                 exhy::GetFiberId(),time(0),exhy::Thread::GetName()))).getSS()

#define EXHY_LOG_DEBUG(logger) EXHY_LOG_LEVEL(logger,exhy::LogLevel::DEBUGs)
#define EXHY_LOG_INFO(logger) EXHY_LOG_LEVEL(logger,exhy::LogLevel::INFO)
#define EXHY_LOG_WARN(logger) EXHY_LOG_LEVEL(logger,exhy::LogLevel::WARN)
#define EXHY_LOG_ERROR(logger) EXHY_LOG_LEVEL(logger,exhy::LogLevel::ERROR)
#define EXHY_LOG_FATAL(logger) EXHY_LOG_LEVEL(logger,exhy::LogLevel::FATAL)

#define EXHY_LOG_FMT_LEVEL(logger,level,fmt,...)\
if(logger->getLevel() <= level) \
    exhy::LogEventWrap(exhy::LogEvent::ptr(new exhy::LogEvent(logger,level, \
                        __FILE__,__LINE__,0,exhy::GetThreadId(),\
                        exhy::GetFiberId(),time(0),exhy::Thread::GetName()))).getEvent()->format(fmt,__VA_ARGS__)
#define EXHY_LOG_FMT_DEBUG(logger,fmt,...) EXHY_LOG_FMT_LEVEL(logger,exhy::LogLevel::DEBUG,fmt,__VA_ARGS__)
#define EXHY_LOG_FMT_INFO(logger,fmt,...) EXHY_LOG_FMT_LEVEL(logger,exhy::LogLevel::INFO,fmt,__VA_ARGS__)
#define EXHY_LOG_FMT_WARN(logger,fmt,...) EXHY_LOG_FMT_LEVEL(logger,exhy::LogLevel::WARN,fmt,__VA_ARGS__)
#define EXHY_LOG_FMT_ERROR(logger,fmt,...) EXHY_LOG_FMT_LEVEL(logger,exhy::LogLevel::ERROR,fmt,__VA_ARGS__)
#define EXHY_LOG_FMT_FATAL(logger,fmt,...) EXHY_LOG_FMT_LEVEL(logger,exhy::LogLevel::FATAL,fmt,__VA_ARGS__)

#define EXHY_LOG_ROOT() exhy::LoggerMgr::GetInstance()->getRoot()
#define EXHY_LOG_NAME(name) exhy::LoggerMgr::GetInstance()->getLogger(name)
namespace exhy{
class Logger;
//日志级别
class LogLevel{
public:
    enum Level{
        UNKNOW = 0,
        DEBUGs = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
    static const char* ToString(LogLevel::Level level);
    static LogLevel::Level FromString(const std::string& str);
};
//日志事件
class LogEvent{
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level
            ,const char* file, int32_t line, uint32_t elapse
            ,uint32_t thread_id, uint32_t fiber_id, uint64_t time
            ,const std::string& thread_name);

    const char* getFile() const {return m_file;}
    int32_t getLine() const {return m_line;}
    uint32_t getElapse() const {return m_elapse;}
    uint32_t getThreadId() const {return m_threadId;}
    uint32_t getFiberId() const {return m_fiberId;}
    uint64_t getTime() const {return m_time;}
    const std::string& getThreadName() const {return m_threadName;}
    std::string getContent() const { return m_ss.str();}
    std::stringstream& getSS(){ return m_ss;}
    std::shared_ptr<Logger> getLogger() const { return m_logger;}
    LogLevel::Level getLevel() const {return m_level;}
     
    void format(const char* fmt, ...);
    void format(const char* fmt,va_list al);
private:
    const char* m_file = nullptr; //文件名
    int32_t m_line = 0;			  //行号
    uint32_t m_elapse = 0;		  //程序启动到现在的毫秒数
    uint32_t m_threadId = 0;	  //线程id
    uint32_t m_fiberId = 0; 	  //协程id
    uint64_t m_time = 0;			  //时间戳
    std::string m_threadName;
    std::stringstream m_ss;
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
};
 
class LogEventWrap{
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();
    LogEvent::ptr getEvent() const { return m_event; }
    std::stringstream& getSS();
private:
    LogEvent::ptr m_event;  
 
};


//日志格式器
class LogFormatter{
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattren);
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event);

    class FormatItem{
    public:
    typedef std::shared_ptr<FormatItem> ptr;
    //FormatItem(const std::string& fmt = "");
    virtual ~FormatItem(){}
    virtual void format(std::ostream& os,std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

    };
    void init();

    bool isError() const {return m_error;}
    const std::string getPattern() const {return m_pattern;}
private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
    bool m_error = false;
};
//日志输出地
class LogAppender{
    friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender(){}
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    virtual std::string toYamlString() = 0;
    
    void setFormatter(LogFormatter:: ptr val);
    LogFormatter::ptr getFormatter();
    LogLevel::Level getLevel() const {return m_level;}
    void setLevel(LogLevel::Level val){m_level = val;}
protected:
    LogLevel::Level m_level = LogLevel::DEBUGs;
    bool m_hasFormatter = false;
    Mutex m_mutex;
    LogFormatter::ptr m_formatter;
};

//日志器
class Logger :public std::enable_shared_from_this<Logger>{
    friend class LoggerManager;
public:
    typedef std::shared_ptr<Logger> ptr;
    Logger(const std::string& name = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);
    void debug(LogEvent:: ptr event);
    void info(LogEvent:: ptr event);
    void warn(LogEvent:: ptr event);
    void error(LogEvent:: ptr event);
    void fatal(LogEvent:: ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppenders();
    
    LogLevel::Level getLevel() const {return m_level;}
    void setLevel(LogLevel::Level val){m_level = val;}
    
    const std::string& getName() const { return m_name;}
    
    void setFormatter(LogFormatter::ptr val);
    void setFormatter(const std::string& val);
    LogFormatter::ptr getFormatter();
    
    std::string toYamlString();
private:
    std::string m_name;		  //日志名称
    LogLevel::Level m_level;  //日志级别
    LogFormatter::ptr m_formatter;
    std::list<LogAppender::ptr> m_appenders; //Appender集合
    Logger::ptr m_root;
    Mutex m_mutex;
};
//输出到控制台的Appender
class StdoutLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    virtual void log(Logger::ptr logger,LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;
};
//输出到文件的Appender
class FileLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    virtual void log(Logger::ptr logger,LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;
    bool reopen();
private:
std::string m_filename;
std::ofstream m_filestream;
};
class LoggerManager{
public:
    LoggerManager();
    Logger::ptr getLogger(const std::string& name);
    Logger::ptr getRoot() const {return m_root;}
    void init();
    std::string toYamlString();
private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
    Mutex m_mutex;
};
typedef exhy::Singleton<LoggerManager> LoggerMgr;
 
 
 
}

#endif
