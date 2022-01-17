#include "exhy/exhy_tcpserver.h"
#include "exhy/exhy_iomanager.h"
#include "exhy/exhy_log.h"

exhy::Logger::ptr g_logger = EXHY_LOG_ROOT();

void run(){
    auto addr = exhy::Address::LookupAny("0.0.0.0:4396");
    std::vector<exhy::Address::ptr> addrs;
    addrs.push_back(addr);
    exhy::TcpServer::ptr tcp_server(new exhy::TcpServer);
    std::vector<exhy::Address::ptr> fails;
    while(!tcp_server->bind(addrs,fails)){
        sleep(2);
    }
    tcp_server->start();
}
int main(int argc, char** argv){
    exhy::IOManager iom(2);
    iom.schedule(run);
    return 0;
}