#include "exhy_fiber.h"
#include "exhy_config.h" 
#include "exhy_macro.h"
#include "exhy_scheduler.h"
#include <atomic>
namespace exhy{

static Logger::ptr g_logger = EXHY_LOG_NAME("system");

static std::atomic<uint64_t> s_fiber_id(0);
static std::atomic<uint64_t> s_fiber_count(0);

static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber::ptr t_threadFiber = nullptr;

static ConfigVar<uint32_t>::ptr g_fiber_stack_size = 
    Config::Lookup<uint32_t>("fiber.stack",1024 * 1024,"fiber stack size");

class MallocStackAllocator{
public:
    static void* Alloc(size_t size){
        return malloc(size);
    }
    static void Dealloc(void* vp,size_t size){
        return free(vp);
    }

};

using StackAllocator = MallocStackAllocator;

Fiber::Fiber(){
    m_state = EXEC;
    SetThis(this);

    if(getcontext(&m_ctx)){
        EXHY_ASSERT2(false,"getcontext failed");
    }
    ++s_fiber_count;

}
Fiber::Fiber(std::function<void()> cb, size_t stack_size,bool use_caller)
    :m_id(++s_fiber_id)
    ,m_cb(cb){
    EXHY_LOG_INFO(g_logger)<<"Fiber::Fiber Created id="<<m_id;
    ++s_fiber_count;
    m_stacksize = stack_size ? stack_size : g_fiber_stack_size->getValue();

    m_stack = StackAllocator::Alloc(m_stacksize);
    if(getcontext(&m_ctx)){
        EXHY_ASSERT2(false,"getcontext failed");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    if(!use_caller){
        makecontext(&m_ctx,Fiber::MainFunc,0);
    }else{
        makecontext(&m_ctx,Fiber::CallerMainFunc,0);
    }
        
    
}
Fiber::~Fiber(){
    --s_fiber_count;
    if(m_stack){
        EXHY_ASSERT(m_state == INIT ||  m_state == TERM || m_state == EXCEP);
        StackAllocator::Dealloc(m_stack,m_stacksize);
    }else{
        EXHY_ASSERT(!m_cb);
        EXHY_ASSERT(m_state == EXEC);

        Fiber* cur = t_fiber;
        if(cur == this){
            SetThis(nullptr);
        }
    }
    EXHY_LOG_INFO(g_logger)<<"Fiber::~Fiber id="<<m_id;
    
}
//设置当前协程
void Fiber::SetThis(Fiber* f){
    t_fiber = f;
}
Fiber::ptr Fiber::GetThis(){
    if(t_fiber){
        return t_fiber->shared_from_this();
    }
    Fiber::ptr main_fiber(new Fiber);
    EXHY_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();

}
//切换到后台，并且设置为Ready状态
void Fiber::YieldToReady(){
    Fiber::ptr cur = GetThis();
    cur->m_state = READY;
    cur->swapOut();
}
//切换到后台，并且设置为Hold状态
void Fiber::YieldToHold(){
    Fiber::ptr cur = GetThis();
    EXHY_ASSERT(cur->m_state == EXEC);
    // cur->m_state = HOLD;
    cur->swapOut();
}
//总协程数
uint64_t Fiber::TotalFibers(){
    return s_fiber_count;
}
void Fiber::MainFunc(){
    Fiber::ptr cur= GetThis();
    EXHY_ASSERT(cur);
    try{
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    }catch(std::exception& ex){
        cur->m_state = EXCEP;
        EXHY_LOG_ERROR(g_logger) << "Fiber Except " << ex.what()
            << " fiber_id=" << cur->getId()
            <<std::endl
            <<exhy::BacktraceToString();
    }catch(...){
        cur->m_state = EXCEP;
        EXHY_LOG_ERROR(g_logger) << "Fiber Except";
    }
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();

    EXHY_ASSERT2(false,"never reach");
}
uint64_t Fiber::GetFiberId(){
    if(t_fiber){
        return t_fiber->getId();
    }
    return 0;
}
void Fiber::CallerMainFunc(){
    Fiber::ptr cur= GetThis();
    EXHY_ASSERT(cur);
    try{
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    }catch(std::exception& ex){
        cur->m_state = EXCEP;
        EXHY_LOG_ERROR(g_logger) << "Fiber Except " << ex.what()
            << " fiber_id=" << cur->getId()
            <<std::endl
            <<exhy::BacktraceToString();
    }catch(...){
        cur->m_state = EXCEP;
        EXHY_LOG_ERROR(g_logger) << "Fiber Except";
    }
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();

    EXHY_ASSERT2(false,"never reach");
}
void Fiber::reset(std::function<void()> cb){
    EXHY_ASSERT(m_stack);
    EXHY_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEP);
    m_cb = cb;
    if(getcontext(&m_ctx)){
        EXHY_ASSERT2(false,"getcontext");
    }    
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx,&Fiber::MainFunc,0);
    m_state = INIT;
}

void Fiber::call(){
    SetThis(this);
    m_state = EXEC;
    if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)){
        EXHY_ASSERT2(false,"swapcontext");
    }
}

void Fiber::back() {
    SetThis(t_threadFiber.get());
    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        EXHY_ASSERT2(false, "swapcontext");
    }
}
//切换到就绪协程执行
void Fiber::swapIn(){
    SetThis(this);
    EXHY_ASSERT(m_state != EXEC);
    m_state = EXEC;
    if(swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)){
        EXHY_ASSERT2(false,"swapcontext");
    }
}
//切换到后台执行
void Fiber::swapOut(){
    // SetThis(t_threadFiber.get());
    SetThis(Scheduler::GetMainFiber());
    if(swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)){
        EXHY_ASSERT2(false,"swapcontext");
    }
}



    
}