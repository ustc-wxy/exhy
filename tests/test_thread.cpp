#include "exhy.h"
#include <unistd.h>

exhy::Logger::ptr g_logger = EXHY_LOG_ROOT();

int count = 0;
//EXHY::RWMutex s_mutex;
exhy::Mutex s_mutex;

void fun1() {
    EXHY_LOG_INFO(g_logger) << "name: " << exhy::Thread::GetName()
                             << " this.name: " << exhy::Thread::GetThis()->getName()
                             << " id: " << exhy::GetThreadId()
                             << " this.id: " << exhy::Thread::GetThis()->getId();

    for(int i = 0; i < 100000; ++i) {
        //exhy::RWMutex::WriteLock lock(s_mutex);
        exhy::Mutex::Lock lock(s_mutex);
        ++count;
    }
}

void fun2() {

}

void fun3() {

}
sem_t bin_sem;
class Sem{
public:
    
    Sem(const std::string& name = "unknow",uint32_t count = 0);
    ~Sem();
private:
    Sem(const Sem&) = delete;
    Sem(const Sem&&) = delete;
    Sem operator = (const Sem) = delete;
private:
    std::string m_name;
    sem_t m_semaphore;
};
Sem::Sem(const std::string& name,uint32_t count):m_name(name){
    int res = sem_init(&bin_sem, 0, 0);
    printf("res = %d\n",res);
    sem_init(&m_semaphore, 0, 0);
}

int main() {
    // exhy::Thread td(new exhy::Thread(&fun2,"tst"));
    // exhy::Thread::ptr t2d(new exhy::Thread(&fun2,"tst"));
    // exhy::Thread::ptr thr(new exhy::Thread(&fun2, "name_"));
    Sem *s  = new Sem();
    







    EXHY_LOG_INFO(g_logger) << "thread test begin";
    // YAML::Node root = YAML::LoadFile("/home/exhy/test/exhy/bin/conf/log2.yml");
    // exhy::Config::LoadFromYaml(root);

    std::vector<exhy::Thread::ptr> thrs;
    for(int i = 0; i < 1; ++i) {
        exhy::Thread::ptr thr(new exhy::Thread(&fun2, "name_" + std::to_string(i * 2)));
        //exhy::Thread::ptr thr2(new exhy::Thread(&fun3, "name_" + std::to_string(i * 2 + 1)));
        thrs.push_back(thr);
        //thrs.push_back(thr2);
    }

    for(size_t i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    }
    EXHY_LOG_INFO(g_logger) << "thread test end";
    EXHY_LOG_INFO(g_logger) << "count=" << count;

    return 0;
}