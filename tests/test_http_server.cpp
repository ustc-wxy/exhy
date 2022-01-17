#include "exhy/http/http_server.h"
#include "exhy/exhy_log.h"

static exhy::Logger::ptr g_logger = EXHY_LOG_ROOT();
std::string s_ip,s_port;
void run(){
    std::cout<<"Please input IP Address:"<<std::endl;
    
    std::cin>>s_ip;

    std::cout<<"Please input Port Number:"<<std::endl;

    std::cin>>s_port;

    std::string add = s_ip+":"+s_port;
    exhy::http::HttpServer::ptr server(new exhy::http::HttpServer);
    exhy::Address::ptr addr = exhy::Address::LookupAnyIPAddress(add);
    while(!server->bind(addr)){
        sleep(5);
    }
    server->start();

    auto sd = server->getServletDispatch();
    std::cout<<"1- add Servlet 2 - add GlobalServlet"<<std::endl;
    int op;
    std::string rt;
    while(std::cin>>op){
        if(op == 1){
            std::cout<<"Please input rt:"<<std::endl;
            std::cin>>rt;
            sd->addServlet(rt, [](exhy::http::HttpRequest::ptr req
                        ,exhy::http::HttpResponse::ptr rsp
                        ,exhy::http::HttpSession::ptr session) {
                    rsp->setBody(req->toString());
                    return 0;
             });
             std::cout<<"Add Servlet "<<rt<<" Success!"<<std::endl;
             std::cout<<"Use url: "<<add<<rt<<"  to visit"<<std::endl;
        }else if(op == 2){
            std::cout<<"Please input rt(G):"<<std::endl;
            std::cin>>rt;
            sd->addGlobServlet(rt, [](exhy::http::HttpRequest::ptr req
                ,exhy::http::HttpResponse::ptr rsp
                ,exhy::http::HttpSession::ptr session) {
            rsp->setBody("Glob:\r\n" + req->toString());
            return 0;
            });
            std::cout<<"Add Global Servlet "<<rt<<" Success!"<<std::endl;
             std::cout<<"Use url: "<<add<<rt<<"  to visit"<<std::endl;
        }
    }
    

    
}

int main(){
    exhy::IOManager iom(2);
    iom.schedule(run);
    return 0;
}