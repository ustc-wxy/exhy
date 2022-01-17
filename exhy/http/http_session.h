#ifndef __EXHY_HTTP_SESSION_H__
#define __EXHY_HTTP_SESSION_H__

#include "exhy/streams/socket_stream.h"
#include "http.h"

namespace exhy{
namespace http{

class HttpSession : public SocketStream{
public:
    typedef std::shared_ptr<HttpSession> ptr;
    HttpSession(Socket::ptr sock, bool owner = true);
    HttpRequest::ptr recvRequest();
    int sendResponse(HttpResponse::ptr rsp);
};

}
}
#endif