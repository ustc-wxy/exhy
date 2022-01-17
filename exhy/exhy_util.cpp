#include "exhy_util.h"
#include "exhy_log.h"
#include "exhy_fiber.h"
#include <execinfo.h>
#include <sys/time.h>

namespace exhy{

exhy::Logger::ptr g_logger = EXHY_LOG_NAME("system");

pid_t GetThreadId(){
    return syscall(SYS_gettid);
}
uint32_t GetFiberId(){
    return 0;
    return exhy::Fiber::GetFiberId();
}
void Backtrace(std::vector<std::string>& bt,int size,int skip){
    void** array = (void**)malloc(sizeof(void*)*size);
    size_t s = ::backtrace(array,size);
    char** strings = backtrace_symbols(array,s);
    if(strings == NULL){
        EXHY_LOG_ERROR(g_logger)<<"backtrace_symbols error";
        return;
    }
    for(size_t i = skip;i < s; ++i){
        bt.push_back(strings[i]);
    }
    free(strings);
    free(array);
}
std::string BacktraceToString(int size,int skip,const std::string& prefix){
    std::vector<std::string> bt;
    Backtrace(bt,size,skip);
    std::stringstream ss;
    //std::cout<<"bt.size()="<<bt.size()<<std::endl;
    for(int i = 0;i < bt.size();i++){
        ss << prefix << bt[i] <<std::endl;
    }
    return ss.str();
}

uint64_t GetCurrentMS(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * 1000ul  + tv.tv_usec / 1000;
}
uint64_t GetCurrentUS(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000ul  + tv.tv_usec;
}

std::string Time2Str(time_t ts, const std::string& format) {
    struct tm tm;
    localtime_r(&ts, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), format.c_str(), &tm);
    return buf;
}

time_t Str2Time(const char* str, const char* format) {
    struct tm t;
    memset(&t, 0, sizeof(t));
    if(!strptime(str, format, &t)) {
        return 0;
    }
    return mktime(&t);
}

}
