#include "exhy_hook.h"
#include "exhy_iomanager.h"
#include "exhy_fdmanager.h"
#include "exhy_fiber.h"
#include "exhy_log.h"
#include "exhy_config.h"
#include "exhy_singleton.h"
#include <dlfcn.h>
#include <vector>
exhy::Logger::ptr g_logger_hook = EXHY_LOG_NAME("system");
namespace exhy{

static exhy::ConfigVar<int>::ptr g_tcp_connect_timeout =
    exhy::Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");

static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

void hook_init(){
    static bool is_inited = false;
    if(is_inited) return;

#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX

}
static uint64_t s_connect_timeout = -1;
struct HookIniter{
    HookIniter(){
        hook_init();

        s_connect_timeout = g_tcp_connect_timeout->getValue();

        g_tcp_connect_timeout->addListener([](const int& old_value, const int& new_value){
                EXHY_LOG_INFO(g_logger_hook) << "tcp connect timeout changed from "
                                         << old_value << " to " << new_value;
                s_connect_timeout = new_value;
        });
    }
};
static HookIniter s_hook_initer;

bool is_hook_enable(){
    return t_hook_enable;
}
void set_hook_enable(bool flag){
    t_hook_enable = flag;
}




}




struct timer_info {
    int cancelled = 0;
};
template<typename OriginFun, typename ... Args>
static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name,
        uint32_t event, int timeout_so, Args&&... args){

    //debug
    // EXHY_LOG_INFO(g_logger_hook)<<" do io<"<<hook_fun_name<<">";

    if(!exhy::t_hook_enable){
        return fun(fd, std::forward<Args>(args)...);
    }
    exhy::FdCtx::ptr ctx = exhy::FdMgr::GetInstance()->get(fd);
    if(!ctx){
        return fun(fd, std::forward<Args>(args)...); 
    }

    if(ctx->isClose()){
        errno = EBADF;
        return -1;
    }
    
    if(!ctx->isSocket() || ctx->getUserNonblock()) {
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = ctx->getTimeout(timeout_so);
    std::shared_ptr<timer_info> tinfo(new timer_info);
retry:
    size_t n = fun(fd, std::forward<Args>(args)...);
    while(n == -1 && errno == EINTR){
        n = fun(fd, std::forward<Args>(args)...);
    }
    if(n == -1 && errno == EAGAIN){
        
        exhy::IOManager* iom = exhy::IOManager::GetThis();
        exhy::Timer::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);

        if(to != (uint64_t)-1) {
            timer = iom->addConditionTimer(to, [winfo, fd, iom, event]() {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, (exhy::IOManager::Event)(event));
            }, winfo);
        }
        int rt = iom->addEvent(fd,(exhy::IOManager::Event)(event));
        if(rt){
            EXHY_LOG_ERROR(g_logger_hook) << hook_fun_name << " addEvent("
                << fd << ", " << event << ")";
            if(timer){
                timer->cancel();
            }
        } else {
            exhy::Fiber::YieldToHold();
            if(timer) {
                timer->cancel();
            }
            if(tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }
            goto retry;
        }

    }

    return n;
}



extern "C" {
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX



unsigned int sleep(unsigned int seconds){
    if(!exhy::t_hook_enable){
        return sleep_f(seconds);
    }
    exhy::Fiber::ptr fiber = exhy::Fiber::GetThis();
    exhy::IOManager* iom = exhy::IOManager::GetThis();
    iom->addTimer(seconds * 1000, [iom,fiber](){
        iom->schedule(fiber);
    });
    // iom->addTimer(seconds * 1000, std::bind((void(exhy::Scheduler::*)
    //         (exhy::Fiber::ptr, int thread))&exhy::IOManager::schedule
    //         ,iom, fiber, -1));
    exhy::Fiber::YieldToHold();
    return 0;
}

int usleep(useconds_t usec){
    if(!exhy::t_hook_enable){
        return usleep_f(usec);
    }
    exhy::Fiber::ptr fiber = exhy::Fiber::GetThis();
    exhy::IOManager* iom = exhy::IOManager::GetThis();
    iom->addTimer(usec / 1000, [iom,fiber](){
        iom->schedule(fiber);
    });
    exhy::Fiber::YieldToHold();
    return 0;
}

int socket(int domain, int type, int protocol){
    int fd = socket_f(domain,type,protocol);
    if(exhy::t_hook_enable && fd != -1){
        exhy::FdMgr::GetInstance()->get(fd,true);
    }
    return fd;
}
int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms) {
    if(!exhy::t_hook_enable) {
        return connect_f(fd, addr, addrlen);
    }
    exhy::FdCtx::ptr ctx = exhy::FdMgr::GetInstance()->get(fd);
    if(!ctx || ctx->isClose()) {
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket()) {
        return connect_f(fd, addr, addrlen);
    }

    if(ctx->getUserNonblock()) {
        return connect_f(fd, addr, addrlen);
    }

    int n = connect_f(fd, addr, addrlen);
    if(n == 0) {
        return 0;
    } else if(n != -1 || errno != EINPROGRESS) {
        return n;
    }

    exhy::IOManager* iom = exhy::IOManager::GetThis();
    exhy::Timer::ptr timer;
    std::shared_ptr<timer_info> tinfo(new timer_info);
    std::weak_ptr<timer_info> winfo(tinfo);

    if(timeout_ms != (uint64_t)-1) {
        timer = iom->addConditionTimer(timeout_ms, [winfo, fd, iom]() {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, exhy::IOManager::WRITE);
        }, winfo);
    }

    int rt = iom->addEvent(fd, exhy::IOManager::WRITE);
    if(rt == 0) {
        exhy::Fiber::YieldToHold();
        if(timer) {
            timer->cancel();
        }
        if(tinfo->cancelled) {
            errno = tinfo->cancelled;
            return -1;
        }
    } else {
        if(timer) {
            timer->cancel();
        }
        EXHY_LOG_ERROR(g_logger_hook) << "connect addEvent(" << fd << ", WRITE) error";
    }

    int error = 0;
    socklen_t len = sizeof(int);
    if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) {
        return -1;
    }
    if(!error) {
        return 0;
    } else {
        errno = error;
        return -1;
    }
}
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    EXHY_LOG_INFO(g_logger_hook)<<"my connect is workong...";
    return connect_with_timeout(sockfd, addr, addrlen, exhy::s_connect_timeout);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen){
    int fd = do_io(s,accept_f,"accept",exhy::IOManager::READ,SO_RCVTIMEO,addr,addrlen);
    if(fd >= 0){
        exhy::FdMgr::GetInstance()->get(fd,true);
    }
    return fd;
}
ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read", exhy::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", exhy::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", exhy::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", exhy::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", exhy::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", exhy::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", exhy::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", exhy::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", exhy::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", exhy::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}
int close(int fd){
    if(!exhy::t_hook_enable){
        return close_f(fd);
    }
    exhy::FdCtx::ptr ctx = exhy::FdMgr::GetInstance()->get(fd);
    if(ctx){
        auto iom = exhy::IOManager::GetThis();
        if(iom){
            iom->cancelAll(fd);
        }
        exhy::FdMgr::GetInstance()->del(fd);
    }
    return close_f(fd);
}
int fcntl(int fd, int cmd, ... /* arg */ ) {
    va_list va;
    va_start(va,cmd);
    switch(cmd){
        case F_SETFL:
            {
                int arg = va_arg(va, int);
                va_end(va);
                exhy::FdCtx::ptr ctx = exhy::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()){
                    return fcntl_f(fd, cmd, arg);
                }
                ctx->setUserNonblock(arg & O_NONBLOCK);
                if(ctx->getSysNonblock()){
                    arg |= O_NONBLOCK;
                }else{
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd,cmd);
                exhy::FdCtx::ptr ctx = exhy::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()){
                    return arg;
                }
                if(ctx->getUserNonblock()){
                    return arg | O_NONBLOCK;
                }else{
                    return arg & ~O_NONBLOCK;
                }
            }
            break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETLEASE:
        case F_NOTIFY:
        case F_SETPIPE_SZ:
            {
                int arg = va_arg(va , int);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
                
            }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
        case F_GETPIPE_SZ:
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);

    }

}

int ioctl(int d, unsigned long int request, ...){
    va_list va;
    va_start(va, request);
    void* arg = va_arg(va, void*);
    va_end(va);

    if(FIONBIO == request) {
        bool user_nonblock = !!*(int*)arg;
        exhy::FdCtx::ptr ctx = exhy::FdMgr::GetInstance()->get(d);
        if(!ctx || ctx->isClose() || !ctx->isSocket()) {
            return ioctl_f(d, request, arg);
        }
        ctx->setUserNonblock(user_nonblock);
    }
    return ioctl_f(d, request, arg);

}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if(!exhy::t_hook_enable) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if(level == SOL_SOCKET) {
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            exhy::FdCtx::ptr ctx = exhy::FdMgr::GetInstance()->get(sockfd);
            if(ctx) {
                const timeval* v = (const timeval*)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);


}






















}