#ifndef __EXHY_MACRO_H__
#define __EXHY_MACRO_H__

#include <string.h>
#include <assert.h>
#include "exhy_util.h"
#include "exhy_log.h"

#if defined __GNUC__ || defined __llvm__
#   define likely(x) __builtin_expect(!!(x), 1) //x很可能为真       
#   define unlikely(x) __builtin_expect(!!(x), 0) //x很可能为假
#else
#   define likely(x)    (x)
#   define unlikely(x)  (x)
#endif



#define EXHY_ASSERT(x) \
    if(!(x)) {  \
        EXHY_LOG_ERROR(EXHY_LOG_ROOT()) << "ASSERTION: "#x \
            <<"\nbacktrace:\n" \
            << exhy::BacktraceToString(100,2,"    "); \
        assert(x); \
    }

#define EXHY_ASSERT2(x,w) \
    if(!(x)) {  \
        EXHY_LOG_ERROR(EXHY_LOG_ROOT()) << "ASSERTION: "#x \
            <<"\n"<<w \
            <<"\nbacktrace:\n" \
            << exhy::BacktraceToString(100,2,"    "); \
        assert(x); \
    }

#endif