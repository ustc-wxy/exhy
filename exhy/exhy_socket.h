#ifndef __EXHY_SOCKET_H__
#define __EXHY_SOCKET_H__

#include <memory>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "exhy_address.h"
#include "exhy_noncopyable.h"

namespace exhy{

/*Socket 封装类*/
class Socket: public std::enable_shared_from_this<Socket>,Noncopyable{
public:
    typedef std::shared_ptr<Socket> ptr;
    typedef std::weak_ptr<Socket> weak_ptr;

    enum Type{
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };

    enum Family{
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX
    };
    /*创建TCP Socket*/
    static Socket::ptr CreateTCP(exhy::Address::ptr address);
    /*创建UDP Socket*/
    static Socket::ptr CreateUDP(exhy::Address::ptr address);
    /*创建IPv4的TCPSocket*/
    static Socket::ptr CreateTCPSocket();
    /*创建IPv4的UDPSocket*/
    static Socket::ptr CreateUDPSocket();
    /*创建IPv6的TCPSocket*/
    static Socket::ptr CreateTCPSocket6();
    /*创建IPv6的UDPSocket*/
    static Socket::ptr CreateUDPSocket6();
    /*创建Unix的TCPSocket*/
    static Socket::ptr CreateUnixTCPSocket();
    /*创建Unix的UDPSocket*/
    static Socket::ptr CreateUnixUDPSocket();
    /*Socket构造函数*/
    Socket(int family, int type, int protocol = 0);
    /*Socket析构函数*/
    virtual ~Socket();
    /*获取发送超时时间*/
    int64_t getSendTimeout();
    /*设置发送超时时间*/
    void setSendTimeout(int64_t v);
    /*获取接收超时时间*/
    int64_t getRecvTimeout();
    /*设置接收超时时间*/
    void setRecvTimeout(int64_t v);
    /*获取Socketopt*/
    bool getOption(int level, int option, void* result, socklen_t* len);
    /*获取Socketopt模板*/
    template<class T>
    bool getOption(int level, int option, T& result){
        socklen_t length = sizeof(T);
        return getOption(level, option, &result, &length);
    }
    /*设置sockopt*/
    bool setOption(int level, int option, const void* result, socklen_t len);
    /*设置sockopt模板*/
    template<class T>
    bool setOption(int level, int option, const T& value){
        return setOption(level, option, &value, sizeof(T));
    }
    /*
        @brief 接收connect连接
        @return 成功返回新连接的socket,失败返回nullptr
        @pre Socket必须bind,listen成功
    */
    virtual Socket::ptr accept();
    /*绑定地址*/
    virtual bool bind(const Address::ptr addr);
    /*连接地址*/
    virtual bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);

    virtual bool reconnect(uint64_t timeout_ms = -1);
    /*监听地址*/
    virtual bool listen(int backlog = SOMAXCONN);
    /*关闭socket*/
    virtual bool close();

    /**
     * @brief 发送数据
     * @param[in] buffer 待发送数据的内存
     * @param[in] length 待发送数据的长度
     * @param[in] flags 标志字
     * @return
     *      @retval >0 发送成功对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int send(const void* buffer, size_t length, int flags = 0);
    /**
     * @brief 发送数据
     * @param[in] buffers 待发送数据的内存(iovec数组)
     * @param[in] length 待发送数据的长度(iovec长度)
     * @param[in] flags 标志字
     * @return
     *      @retval >0 发送成功对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int send(const iovec* buffers, size_t length, int flags = 0);
    /**
     * @brief 发送数据
     * @param[in] buffer 待发送数据的内存
     * @param[in] length 待发送数据的长度
     * @param[in] to 发送的目标地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 发送成功对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0);
    /**
     * @brief 发送数据
     * @param[in] buffers 待发送数据的内存(iovec数组)
     * @param[in] length 待发送数据的长度(iovec长度)
     * @param[in] to 发送的目标地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 发送成功对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags = 0);
    /**
     * @brief 接受数据
     * @param[out] buffer 接收数据的内存
     * @param[in] length 接收数据的内存大小
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int recv(void* buffer, size_t length, int flags = 0);
    /**
     * @brief 接受数据
     * @param[out] buffers 接收数据的内存(iovec数组)
     * @param[in] length 接收数据的内存大小(iovec数组长度)
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int recv(iovec* buffers, size_t length, int flags = 0);
    /**
     * @brief 接受数据
     * @param[out] buffer 接收数据的内存
     * @param[in] length 接收数据的内存大小
     * @param[out] from 发送端地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0);
    /**
     * @brief 接受数据
     * @param[out] buffers 接收数据的内存(iovec数组)
     * @param[in] length 接收数据的内存大小(iovec数组长度)
     * @param[out] from 发送端地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags = 0);
    /*获取远端地址*/
    Address::ptr getRemoteAddress();
    /*获取本地地址*/
    Address::ptr getLocalAddress();
    /*返回socket句柄*/
    int getSocket() const {return m_sock;}
    /*获取协议簇*/
    int getFamily() const {return m_family;}
    /*获取类型*/
    int getType() const {return m_type;}
    /*获取协议*/
    int getProtocol() const {return m_protocol;}
    /*返回是否连接*/
    bool isConnected() const {return m_isConnected;}
    /*是否有效*/
    bool isValid() const;
    /*返回Socket错误*/
    int getError();
    virtual std::ostream& dump(std::ostream& os) const;
    virtual std::string toString() const;
    /*取消读*/
    bool cancelRead();
    /*取消写*/
    bool cancelWrite();
    /*取消accept*/
    bool cancelAccept();
    /*取消所有事件*/
    bool cancelAll();
protected:
    /*初始化socket*/
    void initSock();
    /*创建socket*/
    void newSock();
    /*初始化sock*/
    virtual bool init(int sock);
protected:
    /// scoket 句柄
    int m_sock;
    /// 协议簇
    int m_family;
    /// 类型
    int m_type;
    /// 协议
    int m_protocol;
    /// 是否连接
    bool m_isConnected;
    /// 本地地址
    Address::ptr m_localAddress;
    /// 远端地址
    Address::ptr m_remoteAddress;
};

std::ostream& operator<<(std::ostream& os, const Socket& sock);

}
#endif