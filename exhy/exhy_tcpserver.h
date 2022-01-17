#ifndef __EXHY_TCP_SERVER_H__
#define __EXHY_TCP_SERVER_H__

#include <memory>
#include <functional>
#include "exhy.h"

namespace exhy{

class TcpServer : public std::enable_shared_from_this<TcpServer>,
                    Noncopyable{
public:
    typedef std::shared_ptr<TcpServer> ptr;
    TcpServer(exhy::IOManager* worker = exhy::IOManager::GetThis()
             ,exhy::IOManager* io_worker = exhy::IOManager::GetThis()
             ,exhy::IOManager* accept_worker = exhy::IOManager::GetThis());
    virtual ~TcpServer();    
    virtual bool bind(exhy::Address::ptr addr, bool ssl = false);
    virtual bool bind(const std::vector<Address::ptr>& Address
                    ,std::vector<Address::ptr>& fails
                    ,bool ssl = false);
    virtual bool start();
    virtual void stop();
    uint64_t getRecvTimeout() const { return m_recvTimeout; }
    std::string getName() const { return m_name; }
    void setRecvTimeout(uint64_t v){ m_recvTimeout = v;}
    void setName(const std::string& v){ m_name = v; }

    virtual std::string toString(const std::string& prefix = "");
    std::vector<Socket::ptr> getSocks() const { return m_socks; }
protected:
    virtual void handleClient(Socket::ptr client);
    virtual void startAccept(Socket::ptr sock);
protected:
    std::vector<Socket::ptr> m_socks;
    IOManager* m_worker;
    IOManager* m_ioWorker;
    IOManager* m_acceptWorker;
    uint64_t m_recvTimeout;
    std::string m_name;
    std::string m_type = "tcp";
    bool m_isStop;
    bool m_ssl = false;
};

}
#endif