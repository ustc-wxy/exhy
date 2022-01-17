#include "exhy_thread.h"
#include "exhy_log.h"
#include "exhy_util.h"

namespace exhy {

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";
//system logger
static exhy::Logger::ptr g_logger = EXHY_LOG_NAME("system");

Semaphore::Semaphore(const std::string& name,uint32_t count):m_name(name){
    if(sem_init(&m_semaphore, 0, count)){
        throw std::logic_error("sem_init error");
    }  
}
Semaphore::~Semaphore(){
    sem_close(&m_semaphore);
}

void Semaphore::wait(){
    if(sem_wait(&m_semaphore)){
        throw std::logic_error("sem_wait error");
    }
}
void Semaphore::notify(){
    if(sem_post(&m_semaphore)){
        throw std::logic_error("sem_post error");
    }
    
}

Thread::Thread(std::function<void()> cb,const std::string& name)
    :m_name(name),m_cb(cb){
    
    if(name.empty()) m_name = "UNKNOW";
    // EXHY_LOG_INFO(g_logger)<<"name = "<<name<<" m_name="<<m_name;
    // EXHY_LOG_INFO(g_logger)<<"Thread Init";
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
    if(rt){
        EXHY_LOG_ERROR(g_logger)<<"pthread_create thread fail,rt="<<rt<<" name= "<<name;
        throw std::logic_error("pthread_create error");
    }
    m_semaphore.wait();
}
Thread::~Thread(){
    if(m_thread){
        pthread_detach(m_thread);
    }
}
void* Thread::run(void* arg){
    Thread* thread = (Thread*)arg;
    //std::cout<<"sema is "<<(thread->m_semaphore.m_semaphore)<<std::endl;
    t_thread = thread;
    t_thread_name = thread->m_name;
    thread->m_id = exhy::GetThreadId();
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());
    std::function<void()> cb;
    cb.swap(thread->m_cb);
    thread->m_semaphore.notify();
    cb();
    return 0;
    
}

void Thread::join(){
    if(m_thread){
        int rt = pthread_join(m_thread, nullptr);
        if(rt){
            EXHY_LOG_ERROR(g_logger)<<"pthread_join thread fail,rt="<<rt<<" name= "<<m_name;
            throw std::logic_error("pthread_join error");
        }
    }
}

Thread* Thread::GetThis(){
    return t_thread;
}
const std::string& Thread::GetName(){
    return t_thread_name;
}
void Thread::SetName(const std::string name){
    if(name.empty()) return;
    if(t_thread){
        t_thread->m_name = name;
    }
    t_thread_name = name;
}



}
