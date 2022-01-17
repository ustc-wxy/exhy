#include <iostream>
#include "exhy/http/http_connection.h"
#include "exhy/exhy_log.h"
#include "exhy/http/http_parser.h"
#include "exhy/streams/zlib_stream.h"

static exhy::Logger::ptr g_logger = EXHY_LOG_ROOT();


void test_data() {
    std::string s_host,s_port;
    std::cout<<"Please input host:"<<std::endl;
    std::cin>>s_host;
    std::cout<<"Please input port:"<<std::endl;
    std::cin>>s_port;    

    exhy::Address::ptr addr = exhy::Address::LookupAny(s_host + ":" + s_port);
    auto sock = exhy::Socket::CreateTCP(addr);

    sock->connect(addr);
    const char buff[] = "GET / HTTP/1.1\r\n"
                "connection: close\r\n"
                "Accept-Encoding: gzip, deflate, br\r\n"
                "Host: www.baidu.com\r\n\r\n";
    sock->send(buff, sizeof(buff));

    std::string line;
    line.resize(1024);

    std::ofstream ofs("http.dat", std::ios::binary);
    int total = 0;
    int len = 0;
    while((len = sock->recv(&line[0], line.size())) > 0) {
        total += len;
        ofs.write(line.c_str(), len);
    }
    std::cout << "total: " << total << " tellp=" << ofs.tellp() << std::endl;
    ofs.flush();
}

int main(){
    exhy::IOManager iom(2);
    
    // iom.schedule(test_https);
    iom.schedule(test_data);
    // iom.schedule(test_parser);


    return 0;
}