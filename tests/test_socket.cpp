#include "exhy/exhy.h"

static exhy::Logger::ptr g_logger = EXHY_LOG_ROOT();

void test_socket(){
    exhy::IPAddress::ptr addr = exhy::Address::LookupAnyIPAddress("182.61.200.7");
    if(addr){
        EXHY_LOG_INFO(g_logger)<<"get address: "<<addr->toString();
    }else{
        EXHY_LOG_INFO(g_logger)<<"get address fail";
        return;
    }

    exhy::Socket::ptr sock = exhy::Socket::CreateTCP(addr);
    addr->setPort(80);
    if(!sock->connect(addr)){
        EXHY_LOG_ERROR(g_logger)<<"connect "<<addr->toString()<<" fail"; 
    }else{
        EXHY_LOG_INFO(g_logger)<<"connect "<<addr->toString()<<" success";
    }
    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        EXHY_LOG_INFO(g_logger) << "send fail rt=" << rt;
        return;
    }
    std::string recv_buff;
    recv_buff.resize(4096);
    rt = sock->recv(&recv_buff[0], recv_buff.size(),0);

    if(rt <= 0) {
        EXHY_LOG_INFO(g_logger) << "recv fail rt=" << rt;
        return;
    }
    recv_buff.resize(rt);
    EXHY_LOG_INFO(g_logger)<<recv_buff;
}
int main(){
    exhy::IOManager iom;
    iom.schedule(&test_socket);
    return 0;
}