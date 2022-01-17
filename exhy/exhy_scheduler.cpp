#include "exhy_scheduler.h"
#include "exhy_log.h"
#include "exhy_macro.h"
#include "exhy_hook.h"
namespace exhy{
static exhy::Logger::ptr g_logger = EXHY_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_fiber = nullptr;

Scheduler::Scheduler(size_t threads,bool use_caller,const std::string& name)
    :m_name(name){
    EXHY_ASSERT(threads > 0);

    if(use_caller){
        exhy::Fiber::GetThis();
        --threads;

        EXHY_ASSERT(GetThis() == nullptr);
        t_scheduler = this;
        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run,this),0,true));
        exhy::Thread::SetName(m_name);
        t_fiber = m_rootFiber.get();
        m_rootThread = exhy::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    }else{
        m_rootThread = -1;
    }
    m_threadCount = threads;
}


Scheduler::~Scheduler(){
    EXHY_ASSERT(m_stopping);
    if(GetThis() == this){
        t_scheduler = nullptr;
    }
}
Scheduler* Scheduler::GetThis(){
    return t_scheduler;
}
Fiber* Scheduler::GetMainFiber(){
    return t_fiber;
} 
void Scheduler::start(){
    MutexType::Lock lock(m_mutex);
    if(!m_stopping){
        return;
    }
    m_stopping = false;
    EXHY_ASSERT(m_threads.empty());
    m_threads.resize(m_threadCount);
    for(size_t i = 0; i < m_threadCount; i++){
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run,this)
        ,m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();
    // if(m_rootFiber){
    //     // m_rootFiber->swapIn();
    //     m_rootFiber->call();
    //     EXHY_LOG_INFO(g_logger)<<"call out "<<m_rootFiber->getState();
    // }
}
void Scheduler::stop(){
    m_autoStop = true;
    if(m_rootFiber 
        && m_threadCount == 0
        && (m_rootFiber->getState() == Fiber::TERM
        || m_rootFiber->getState() == Fiber::INIT)){
        EXHY_LOG_INFO(g_logger)<<this<<" stopped";
        m_stopping = true;

        if(stopping()) return;
    }
    if(m_rootThread != -1) {
        EXHY_ASSERT(GetThis() == this);
    } else {
        EXHY_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();
    }

    if(m_rootFiber) {
        tickle();
    }
    if(m_rootFiber) {
        EXHY_LOG_INFO(g_logger)<<"stop";
        //while(!stopping()) {
        //    if(m_rootFiber->getState() == Fiber::TERM
        //            || m_rootFiber->getState() == Fiber::EXCEPT) {
        //        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        //        EXHY_LOG_INFO(g_logger) << " root fiber is term, reset";
        //        t_fiber = m_rootFiber.get();
        //    }
        //    m_rootFiber->call();
        //}
        if(!stopping()) {
            m_rootFiber->call();
        }
    }
}
void Scheduler::SetThis(){
    t_scheduler = this;
}
void Scheduler::run(){
    EXHY_LOG_INFO(g_logger)<<"run";
    set_hook_enable(true);
    SetThis();
    if(exhy::GetThreadId() != m_rootThread){
        t_fiber = Fiber::GetThis().get();
    }
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle,this)));
    Fiber::ptr cb_fiber;

    FiberAndThread ft;
    while(true){
        ft.reset();
        //EXHY_ASSERT(ft.cb == nullptr); 
        bool tickle_me = false;
        bool is_active = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while(it != m_fibers.end()){
                if(it->thread != -1 && it->thread != exhy::GetThreadId()){
                    it++;
                    tickle_me = true;
                    continue;
                }
                EXHY_ASSERT(it->cb || it->fiber);
                if(it->fiber && it->fiber->getState() == Fiber::EXEC) {
                        ++it;
                        continue;
                    }

                    ft = *it;
                    m_fibers.erase(it++);
                    ++m_activeThreadCount;
                    is_active = true;
                    break;

            }
            tickle_me |= it != m_fibers.end();
        }
        if(tickle_me) {
            tickle();
        }
        //EXHY_ASSERT(ft.cb == nullptr); 
        /*RE-write*/
        Fiber::State st;
        if(ft.fiber){
            st = ft.fiber->getState();
            if(st != Fiber::TERM && st != Fiber::EXCEP){
                ft.fiber->swapIn();
                --m_activeThreadCount;
                st = ft.fiber->getState();

                if(st == Fiber::READY){
                    schedule(ft.fiber);
                }else if(st != Fiber::TERM && st != Fiber::EXCEP){
                    ft.fiber->m_state = Fiber::HOLD;
                }
                ft.reset();

            }

        }else if(ft.cb != nullptr){
            if(cb_fiber){
                cb_fiber->reset(ft.cb);
            }else{
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            cb_fiber->swapIn();
            --m_activeThreadCount;
            st = cb_fiber->getState();
            if(st == Fiber::READY){
                schedule(cb_fiber);
                cb_fiber.reset();
            }else if(st == Fiber::TERM || st == Fiber::EXCEP){
                cb_fiber->reset(nullptr);
            }else{
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset();
            }
        }else{
            if(is_active){
                --m_activeThreadCount;
                continue;
            }
            if(idle_fiber->getState() == Fiber::TERM){
                EXHY_LOG_INFO(g_logger)<< "idle fiber term";
                break;
            }
            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            st = idle_fiber->getState();
            if(st != Fiber::TERM && st != Fiber::EXCEP){
                idle_fiber->m_state = Fiber::HOLD;
            }

            }

    
    }

}
void Scheduler::tickle(){
    EXHY_LOG_INFO(g_logger)<<"tickle";
}

bool Scheduler::stopping(){
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping && m_fibers.empty() && m_activeThreadCount == 0;
}
void Scheduler::idle() {
    EXHY_LOG_INFO(g_logger) << "idle";
    while(!stopping()) {
        exhy::Fiber::YieldToHold();
    }
}



}