#include "exhy/exhy.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
exhy::Logger::ptr g_logger = EXHY_LOG_ROOT();
int sock = 0;
void test_fiber(){
    EXHY_LOG_INFO(g_logger)<<"test_fiber_in_iom";
}
void test_1(){
    exhy::IOManager iom(2, true);
    // exhy::IOManager iom;
    iom.schedule(&test_fiber);

    sock = socket(AF_INET, SOCK_STREAM, 0); 
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "182.61.200.7", &addr.sin_addr.s_addr);


    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))){
        // connext success
    }else if(errno == EINPROGRESS){
        EXHY_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        exhy::IOManager::GetThis()->addEvent(sock, exhy::IOManager::READ,[](){
            EXHY_LOG_INFO(g_logger)<<"read callback";
        });
        exhy::IOManager::GetThis()->addEvent(sock, exhy::IOManager::WRITE,[](){
            EXHY_LOG_INFO(g_logger)<<"write callback";
            exhy::IOManager::GetThis()->cancelEvent(sock, exhy::IOManager::READ);
            close(sock);
        });

    }else{
        EXHY_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
    
}
exhy::Timer::ptr s_timer;
void test_timer(){
    exhy::IOManager iom;

    s_timer = iom.addTimer(500,[](){
        static int i = 0;
        EXHY_LOG_INFO(g_logger)<<"hello timer i = "<<i;
        
        if(++i == 5){
            //  s_timer->reset(2000, true);
            s_timer->cancel();
        }
    },true);
}
int main(){
    test_timer();
    // test_1();
    return 0;
}