#ifndef __EXHY_HTTP_SERVLET_H__
#define __EXHY_HTTP_SERVLET_H__

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include "http.h"
#include "http_session.h"
#include "exhy/exhy_thread.h"
#include "exhy/exhy_util.h"

namespace exhy{
namespace http{

class Servlet{
public:
    typedef std::shared_ptr<Servlet> ptr;

    Servlet(const std::string& name)
        :m_name(name){}
    virtual ~Servlet(){}
    virtual int32_t handle(exhy::http::HttpRequest::ptr request
                          ,exhy::http::HttpResponse::ptr response
                          ,exhy::http::HttpSession::ptr session) =0;
    const std::string& getName() const { return m_name; }
protected:
    std::string m_name;
};

class FunctionServlet : public Servlet{
public:
    typedef std::shared_ptr<FunctionServlet> ptr;
    typedef std::function<int32_t (exhy::http::HttpRequest::ptr request
                   , exhy::http::HttpResponse::ptr response
                   , exhy::http::HttpSession::ptr session)> callback;
    FunctionServlet(callback cb);
    virtual int32_t handle(exhy::http::HttpRequest::ptr request
                   , exhy::http::HttpResponse::ptr response
                   , exhy::http::HttpSession::ptr session) override;
private:
    callback m_cb;
};

class NotFoundServlet : public Servlet {
public:
    typedef std::shared_ptr<NotFoundServlet> ptr;    NotFoundServlet(const std::string& name);
    virtual int32_t handle(exhy::http::HttpRequest::ptr request
                   , exhy::http::HttpResponse::ptr response
                   , exhy::http::HttpSession::ptr session) override;

private:
    std::string m_name;
    std::string m_content;
};

class ServletDispatch : public Servlet {
public:
    typedef std::shared_ptr<ServletDispatch> ptr;
    typedef RWMutex RWMutexType;

    ServletDispatch();
    virtual int32_t handle(exhy::http::HttpRequest::ptr request
                   , exhy::http::HttpResponse::ptr response
                   , exhy::http::HttpSession::ptr session) override;

    void addServlet(const std::string& uri, Servlet::ptr slt);
    void addServlet(const std::string& uri, FunctionServlet::callback cb);
    void addGlobServlet(const std::string& uri, Servlet::ptr slt);
    void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);

    // void addServletCreator(const std::string& uri, IServletCreator::ptr creator);
    // void addGlobServletCreator(const std::string& uri, IServletCreator::ptr creator);

    // template<class T>
    // void addServletCreator(const std::string& uri) {
    //     addServletCreator(uri, std::make_shared<ServletCreator<T> >());
    // }

    // template<class T>
    // void addGlobServletCreator(const std::string& uri) {
    //     addGlobServletCreator(uri, std::make_shared<ServletCreator<T> >());
    // }

    void delServlet(const std::string& uri);
    void delGlobServlet(const std::string& uri);

    Servlet::ptr getDefault() const { return m_default;}
    void setDefault(Servlet::ptr v) { m_default = v;}


    Servlet::ptr getServlet(const std::string& uri);
    Servlet::ptr getGlobServlet(const std::string& uri);
    Servlet::ptr getMatchedServlet(const std::string& uri);

    // void listAllServletCreator(std::map<std::string, IServletCreator::ptr>& infos);
    // void listAllGlobServletCreator(std::map<std::string, IServletCreator::ptr>& infos);
private:
    RWMutexType m_mutex;
    std::unordered_map<std::string, Servlet::ptr> m_datas;
    std::vector<std::pair<std::string, Servlet::ptr> > m_globs;
    Servlet::ptr m_default;
};

}
}
#endif