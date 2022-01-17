#include "exhy.h"

exhy::Logger::ptr g_logger = EXHY_LOG_ROOT();

void test_fiber(){
    static int s_count = 5;
    EXHY_LOG_INFO(g_logger)<<"test in fiber s_count = "<<s_count;
    if(--s_count > 0 ){
        exhy::Scheduler::GetThis()->schedule(&test_fiber);
    }
}
int main(){
    EXHY_LOG_INFO(g_logger)<<"main start";
    // exhy::Scheduler sc(3,true,"test");
    exhy::Scheduler sc;
    sc.start();
    sc.schedule(&test_fiber);
    sc.stop();
    EXHY_LOG_INFO(g_logger)<<"main over";
    return 0;
}