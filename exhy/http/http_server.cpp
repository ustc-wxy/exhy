#include "http_server.h"
#include "exhy/exhy_log.h"

namespace exhy{
namespace http{

static exhy::Logger::ptr g_logger = EXHY_LOG_NAME("system");
HttpServer::HttpServer(bool keepalive
            ,exhy::IOManager* worker 
            ,exhy::IOManager* accept_worker)
    :TcpServer(worker, accept_worker)
    ,m_isKeepalive(keepalive){
    m_dispatch.reset(new ServletDispatch);
}

void HttpServer::handleClient(Socket::ptr client){
    HttpSession::ptr session(new HttpSession(client));
    do{
        auto req = session->recvRequest();
        if(!req){
            EXHY_LOG_INFO(g_logger)<<"recv http request fail, errno="
                << errno <<" errstr="<<strerror(errno)
                <<" clinet:"<< *client;
            break;
        }
        HttpResponse::ptr rsp(new HttpResponse(req->getVersion()
                            ,req->isClose() || !m_isKeepalive));
        rsp->setBody("hello wxy 2021 11 20");

        m_dispatch->handle(req,rsp,session);

        session->sendResponse(rsp);
        
        if(!m_isKeepalive || req->isClose()) {
            break;
        }

    }while(true);
    session->close();
}


}
}