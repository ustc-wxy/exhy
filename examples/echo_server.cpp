#include "exhy/exhy_tcpserver.h"
#include "exhy/exhy_log.h"
#include "exhy/exhy_iomanager.h"
#include "exhy/exhy_bytearray.h"
#include "exhy/exhy_address.h"

static exhy::Logger::ptr g_logger = EXHY_LOG_ROOT();

class EchoServer : public exhy::TcpServer {
public:
    EchoServer(int type);
    void handleClient(exhy::Socket::ptr client);

private:
    int m_type = 0;
};

EchoServer::EchoServer(int type)
    :m_type(type) {
}

void EchoServer::handleClient(exhy::Socket::ptr client) {
    EXHY_LOG_INFO(g_logger) << "handleClient " << *client;   
    exhy::ByteArray::ptr ba(new exhy::ByteArray);
    while(true) {
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);

        int rt = client->recv(&iovs[0], iovs.size());
        if(rt == 0) {
            EXHY_LOG_INFO(g_logger) << "client close: " << *client;
            break;
        } else if(rt < 0) {
            EXHY_LOG_INFO(g_logger) << "client error rt=" << rt
                << " errno=" << errno << " errstr=" << strerror(errno);
            break;
        }
        ba->setPosition(ba->getPosition() + rt);
        ba->setPosition(0);
        // EXHY_LOG_INFO(g_logger) << "recv rt=" << rt << " data=" << std::string((char*)iovs[0].iov_base, rt);
        
        if(m_type == 1) {//text 
            std::cout << ba->toString();// << std::endl;
        } else {
            std::cout << ba->toHexString();// << std::endl;
        }
        std::cout.flush();
    }
}

int type = 1;

void run() {
    EXHY_LOG_INFO(g_logger) << "server type=" << type;
    EchoServer::ptr es1(new EchoServer(type));
    EchoServer::ptr es2(new EchoServer(type));

    // auto addr1 = exhy::Address::LookupAny("0.0.0.0:8020");
    // while(!es1->bind(addr1)) {
    //     sleep(2);
    // }
    // es1->start();

    auto addr2 = exhy::Address::LookupAny("0.0.0.0:8030");
    while(!es2->bind(addr2)) {
        sleep(2);
    }
    es2->start();
}

int main(int argc, char** argv) {
    if(argc < 2) {
        EXHY_LOG_INFO(g_logger) << "used as[" << argv[0] << " -t] or [" << argv[0] << " -b]";
        return 0;
    }

    if(!strcmp(argv[1], "-b")) {
        type = 2;
    }

    exhy::IOManager iom(10);
    iom.schedule(run);
    return 0;
}
