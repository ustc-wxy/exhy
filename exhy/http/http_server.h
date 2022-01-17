#ifndef __EXHY_HTTP_SERVER_H__
#define __EXHY_HTTP_SERVER_H__

#include "exhy/exhy_tcpserver.h"
#include "exhy/http/http_servlet.h"
#include "http_session.h"

namespace exhy{
namespace http{

class HttpServer : public TcpServer{
public:
    typedef std::shared_ptr<HttpServer> ptr;
    HttpServer(bool keepalive = false
            ,exhy::IOManager* worker = exhy::IOManager::GetThis()
            ,exhy::IOManager* accept_worker = exhy::IOManager::GetThis());

    ServletDispatch::ptr getServletDispatch() const { return m_dispatch;}
    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v;}
protected:
    virtual void handleClient(Socket::ptr client) override;
private:
    bool m_isKeepalive;        
    /// Servlet分发器
    ServletDispatch::ptr m_dispatch;
};


}
}
#endif