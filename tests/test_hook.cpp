#include "exhy/exhy.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
exhy::Logger::ptr g_logger = EXHY_LOG_ROOT();
void test_sleep(){
    exhy::IOManager iom(1);
    iom.schedule([](){
        sleep(2);
        EXHY_LOG_INFO(g_logger)<<"sleep 2";
    });

    iom.schedule([](){
        sleep(3);
        EXHY_LOG_INFO(g_logger)<<"sleep 3";
    });
    EXHY_LOG_INFO(g_logger)<<"test sleep ";

}
void test_sock(){
    int sock = socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "182.61.200.7", &addr.sin_addr.s_addr);

    int rt = connect(sock,(const sockaddr*)&addr,sizeof(addr));
    EXHY_LOG_INFO(g_logger)<<"con rt = "<<rt<<"  errno = "<<errno;
    if(rt) return;

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock,data,sizeof(data),0);
    EXHY_LOG_INFO(g_logger)<<"send rt = "<<rt<<"  errno = "<<errno;
    if(rt <= 0) return;

    std::string buff;
    buff.resize(4096);

    rt = recv(sock,&buff[0],buff.size(),0);
    EXHY_LOG_INFO(g_logger)<<"recv rt = "<<rt<<"  errno = "<<errno;
    if(rt <= 0) return;
    EXHY_LOG_INFO(g_logger)<<buff;
}
int main(){
    // test_sleep();
    test_sock();
    return 0;
}