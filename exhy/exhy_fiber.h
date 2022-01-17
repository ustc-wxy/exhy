#ifndef __EXHY_FIBER_H__
#define __EXHY_FIBER_H__

#include <memory>
#include <ucontext.h>
#include <functional>
#include "exhy_thread.h"

namespace exhy{
/*
Thread -> main_fiber <---> sub_fiber
*/
class Fiber : public std::enable_shared_from_this<Fiber>{
friend class Scheduler;
public:
    typedef std::shared_ptr<Fiber> ptr;
    enum State{
        INIT,
        HOLD,
        EXEC,
        TERM,
        READY,
        EXCEP
    };
    Fiber(std::function<void()> cb, size_t stack_size = 0,bool use_caller = false);
    ~Fiber();
    static void SetThis(Fiber* f);
    static Fiber::ptr GetThis();
    static void YieldToReady();
    static void YieldToHold();
    static uint64_t TotalFibers();
    static void MainFunc();
    static void CallerMainFunc();
    static uint64_t GetFiberId();
    void reset(std::function<void()> cb);
    void call();
    void back();
    void swapIn();
    void swapOut();
    State getState() const { return m_state;}
    uint64_t getId() const {return m_id;}
private:
    Fiber();
    uint64_t m_id = 0;
    uint32_t m_stacksize = 0;
    State m_state = INIT;

    ucontext_t m_ctx;
    void* m_stack = nullptr;

    std::function<void()> m_cb;
};


}

#endif