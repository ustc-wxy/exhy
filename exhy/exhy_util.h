#ifndef __EXHY_UTIL_H__
#define __EXHY_UTIL_H__
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
namespace exhy{

pid_t GetThreadId();
uint32_t GetFiberId();
void Backtrace(std::vector<std::string>& bt,int size = 64,int skip = 1);
std::string BacktraceToString(int size = 64,int skip = 2,const std::string& prefix = " ");
// time ms us
uint64_t GetCurrentMS();
uint64_t GetCurrentUS();

std::string Time2Str(time_t ts, const std::string& format);
time_t Str2Time(const char* str, const char* format);

}



#endif // __EXHY_UTIL_H__
