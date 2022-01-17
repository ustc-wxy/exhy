#ifndef __EXHY_TIMER_H__
#define __EXHY_TIMER_H__

#include <memory>
#include <set>
#include <vector>
#include "exhy_thread.h"
namespace exhy{

class TimerManager;
class Timer : public std::enable_shared_from_this<Timer>{   
friend class TimerManager;
public:
    typedef std::shared_ptr<Timer> ptr;
    bool cancel();
    bool refresh();
    bool reset(uint64_t ms, bool from_now);
private:
    Timer(uint64_t ms, std::function<void()> cb, 
        bool recurring, TimerManager* manager);
    Timer(uint64_t next);

private:
    bool m_recurring = false;
    uint64_t m_ms = 0;
    uint64_t m_next = 0;
    TimerManager* m_manager = nullptr;
    std::function<void()> m_cb;
private:
    struct Comparator{
        bool operator () (const Timer::ptr& lhs,const Timer::ptr& rhs) const;
    };
};

class TimerManager{
friend class Timer;
public:
    typedef RWMutex RWMutexType;

    TimerManager();
    virtual ~TimerManager();

    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb
                        ,bool recurring = false);

    Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb
                        ,std::weak_ptr<void> weak_cond
                        ,bool recurring = false);
    uint64_t getNextTimer();
    void listExpiredCb(std::vector<std::function<void()> >& cbs);
    bool hasTimer();
protected:
    virtual void onTimerInsertedAtFront() = 0;
    void addTimer(Timer::ptr val, RWMutexType::WriteLock& lock);
    
private:
    bool detectClockRollOver(uint64_t now_ms);
private:
    RWMutexType m_mutex;
    std::set<Timer::ptr, Timer::Comparator> m_timers;
    bool m_tickled = false;
    uint64_t m_previouseTime = 0;
};






}


#endif