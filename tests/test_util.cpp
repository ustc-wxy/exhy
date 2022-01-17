#include "exhy_util.h"
#include "exhy_log.h"
#include "exhy_macro.h"
#include "exhy_fiber.h"
#include <iostream>
#include <assert.h>
using namespace std;

exhy::Logger::ptr g_logger = EXHY_LOG_ROOT();
void test_assert(){
    EXHY_LOG_INFO(g_logger)<< exhy::BacktraceToString(10,2," ");
    EXHY_ASSERT2(0==1,"test for 0 == 1");
}
int main(){
    
    test_assert();
    cout<<"Util Test Start:"<<endl;
    return 0;
}