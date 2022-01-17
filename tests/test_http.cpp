#include "exhy/http/http.h"
#include "exhy/exhy.h"

void test_request(){
    exhy::http::HttpRequest::ptr req(new exhy::http::HttpRequest);
    req->setHeader("host","www.baidu.com");
    req->setBody("hello,this is a test message");

    req->dump(std::cout)<<std::endl;
}
void test_response(){
    exhy::http::HttpResponse::ptr rsp(new exhy::http::HttpResponse);
    rsp->setHeader("X-X","wxy");
    rsp->setBody("hello wxy");
    rsp->setCookie("price","100");
    rsp->setStatus((exhy::http::HttpStatus)400);
    rsp->setClose(false);

    rsp->dump(std::cout)<<std::endl;
}
int main(int ragc, char** argv){
    // test_request();
    test_response();
    return 0;
}
