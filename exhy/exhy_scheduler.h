#ifndef __EXHY_SCHEDULER_H__
#define __EXHY_SCHEDULER_H__

#include <memory>
#include <list>
#include "exhy_fiber.h"
#include "exhy_log.h"
#include "exhy_thread.h"

namespace exhy{

class Scheduler{
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;
    Scheduler(size_t threads = 1,bool use_caller = true,const std::string& name = "");
    virtual ~Scheduler();
    const std::string& getName() const { return m_name; }
    static Scheduler* GetThis();
    static Fiber* GetMainFiber(); 
    void start();
    void stop();
    bool hasIdleThread(){ return m_idleThreadCount > 0; }
    
    template<class FiberOrCb>
    void schedule(FiberOrCb fc, int  thread = -1){
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNolock(fc,thread);
        }
        if(need_tickle){
            tickle();
        }
        
    }

    template<class InputIterator>
    void schedule(InputIterator begin, InputIterator end){
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while(begin != end){
                need_tickle = scheduleNolock(&*begin,-1) || need_tickle;
                ++ begin;
            }
        }
        if(need_tickle){
            tickle();
        }
    }
protected:
    virtual void tickle();
    virtual void idle();
    virtual bool stopping();
    void run();
    void SetThis();
    
private:
    //无锁schedule
    template<class FiberOrCb>
    bool scheduleNolock(FiberOrCb fc, int thread){
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fc,thread);
        if(ft.cb || ft.fiber){
            m_fibers.push_back(ft);
        }
        return need_tickle;
    }
private:
    struct FiberAndThread{
        /* data */
        Fiber::ptr fiber;
        std::function<void()> cb;
        int thread;

        FiberAndThread(Fiber::ptr f,int thr)
            :fiber(f),thread(thr){
        }
        FiberAndThread(Fiber::ptr *f,int thr)
            :thread(thr){
            fiber.swap(*f);
        }
        FiberAndThread(std::function<void()> f,int thr)
            :cb(f),thread(thr){
        }
        FiberAndThread(std::function<void()> *f,int thr)
            :thread(thr){
            cb.swap(*f);
        }
        FiberAndThread()
            :thread(-1){
        }
        void reset(){
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    };
    
private:
    MutexType m_mutex;
    //线程池
    std::vector<Thread::ptr> m_threads;
    //待执行的协程队列 
    std::list<FiberAndThread> m_fibers;
    std::string m_name;
    //调度协程
    Fiber::ptr m_rootFiber;
protected:
    std::vector<int> m_threadIds;
    size_t m_threadCount = 0;
    size_t m_activeThreadCount = 0;
    size_t m_idleThreadCount = 0;
    bool m_stopping = true;
    bool m_autoStop = false;
    int m_rootThread = 0;
};


}


#endif