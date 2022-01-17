#include "exhy/exhy.h"
using namespace std;
exhy::Logger::ptr g_logger = EXHY_LOG_ROOT();
void run_in_fiber(){
    EXHY_LOG_INFO(g_logger)<<"run_in_fiber begin";
    exhy::Fiber::YieldToHold();
    EXHY_LOG_INFO(g_logger)<<"run_in_fiber end";
    exhy::Fiber::YieldToHold();
}
void test_fiber(){
    exhy::Fiber::GetThis();
    EXHY_LOG_INFO(g_logger)<<"main begin";
    exhy::Fiber::ptr fiber(new exhy::Fiber(run_in_fiber));
    EXHY_LOG_INFO(g_logger)<<"------------------";
    fiber->swapIn();
    EXHY_LOG_INFO(g_logger)<<"==================";
    fiber->swapIn();
    EXHY_LOG_INFO(g_logger)<<"main end1";
    fiber->swapIn();
    EXHY_LOG_INFO(g_logger)<<"main end2";
}
int main(){
    exhy::Thread::SetName("main_thread:");
    test_fiber();
    std::vector<exhy::Thread::ptr> thrs;
    for(int i=0;i<3;i++){
        EXHY_LOG_INFO(g_logger)<<i;
        exhy::Thread::ptr td(new exhy::Thread(&test_fiber,"name"+std::to_string(i)));
        thrs.push_back(td);
    }
    for(auto i: thrs){
        i->join();
    }
    return 0;
}