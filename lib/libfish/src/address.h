#pragma once

#include "common.h"
#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>

FISH_NAMESPACE_START

class IPAddress;

/**
 * @brief 网络地址抽象类
 */
class Address {
public:
    typedef std::shared_ptr<Address> Ptr;

    /**
     * @brief 创建地址类
     * @param[in] addr sockaddr地址
     * @return 返回和sockaddr相匹配的Address,失败返回nullptr
     */
    static Address::Ptr create(const sockaddr &addr);

    /**
     * @brief       解析host返回符合条件的地址对象指针
     * @param[out]  result 返回解析结果
     * @param[in]   host 解析目标主机
     * @param[in]   family 协议族
     * @param[in]   type UDP和TCP的IO类型
     * @param[in]   protocol 传输层协议
     * @return      是否解析成功
     */
    static bool lookup(std::vector<Address::Ptr>& result, const std::string& host, int family = AF_INET, int type = 0, int protocol = 0);
    /**
     * @brief       解析host地址返回一个地址
     * @param[in]   host 解析目标主机
     * @param[in]   family 协议族
     * @param[in]   type UDP和TCP的IO类型
     * @param[in]   protocol 传输层协议
     * @return      解析结果, 失败则返回nullptr
     */
    static Address::Ptr lookup_any(const std::string& host, int family = AF_INET, int type = 0, int protocol = 0);

    /**
     * @brief       解析host地址返回一个地址
     * @param[in]   host 解析目标主机
     * @param[in]   family 协议族
     * @param[in]   type UDP和TCP的IO类型
     * @param[in]   protocol 传输层协议
     * @return      解析结果, 失败则返回nullptr
     */
    static std::shared_ptr<IPAddress> lookup_any_ipaddress(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);

    /**
     * @brief               返回本机网卡<网卡名, 地址, 子网掩码位数>
     * @param[out]result    结果
     * @param[in] family    协议族
     * @return              是否成功
     */
    static bool get_interface_addresses(std::multimap<std::string ,std::pair<Address::Ptr, uint32_t> >& result, int family = AF_INET);

    /**
     * @brief               返回本机网卡<网卡名, 地址, 子网掩码位数>
     * @param[out]result    结果
     * @param[in] family    协议族
     * @return              是否成功
     */
    static bool get_interface_addresses(std::vector<std::pair<Address::Ptr, uint32_t> >&result ,const std::string& iface, int family = AF_INET);

    virtual ~Address() {}


    /**
     * @brief 获取地址
     */
    virtual const sockaddr* get_addr() const = 0;

    /**
     * @brief 获取协议簇
     */
    int get_family() const;

    /**
     * @brief 获取可写指针
     */
    virtual sockaddr* get_addr() = 0;

    /**
     * @brief 获取长度
     */
    virtual socklen_t size() const = 0;

    /**
     * @brief 可读性输出地址
     */
    virtual std::ostream& insert(std::ostream& os) const = 0;

    /**
     * @brief 获取可读性字符串
     */
    std::string to_string() const;

    /**
     * @brief 小于号比较函数
     */
    bool operator<(const Address& rhs) const;

    /**
     * @brief 等于函数
     */
    bool operator==(const Address& rhs) const;

    /**
     * @brief 不等于函数
     */
    bool operator!=(const Address& rhs) const;
};

/**
 * @brief IP地址的基类
 */
class IPAddress : public Address {
public:
    typedef std::shared_ptr<IPAddress> Ptr;

    
    /**
     * @brief 创建地址类
     * @param[in] addr sockaddr地址
     * @return 返回和sockaddr相匹配的Address,失败返回nullptr
     */
    static IPAddress::Ptr create(const char* address, uint16_t port = 0);

    /**
     * @brief                   根据本地址获取广播地址
     * @param[in] prefix_len    子网掩码位数
     * @return                  失败则返回nullptr
     */
    virtual IPAddress::Ptr broadcast(uint32_t prefix_len) = 0;

    /**
     * @brief                   获取该地址所对应的网络号
     * @param[in] prefix_len    子网掩码数
     * @return                  失败则返回nullptr
     */
    virtual IPAddress::Ptr network(uint32_t prefix_len) = 0;

    /**
     * @brief                   获取子网掩码(IP地址形式)
     * @param[in] prefix_len    子网掩码数
     * @return                  失败则返回nullptr
     */
    virtual IPAddress::Ptr mask(uint32_t prefix_len) = 0;

    /**
     * @brief 返回端口号
     */
    virtual uint32_t port() const = 0;

    /**
     * @brief 设置端口号
     */
    virtual void set_port(uint16_t v) = 0;
};

/**
 * @brief IPv4地址
 */
class IPv4Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv4Address> ptr;

    /**
     * @brief
     * @param[in] address   点分十进制地址
     * @param[in] port      端口号
     * @return              失败返回nullptr
     */
    static IPv4Address::Ptr create(const char* address, uint16_t port = 0);

    /**
     * @brief   构造IPv4Address
     * @param[in] address
     */
    IPv4Address(const sockaddr_in& address);

    /**
     * @brief 通过二进制地址构造IPv4Address
     * @param[in] address 二进制地址address
     * @param[in] port 端口号
     */
    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

    const sockaddr* get_addr() const override;
    sockaddr* get_addr() override;
    socklen_t size() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress::Ptr broadcast(uint32_t prefix_len) override;
    IPAddress::Ptr network(uint32_t prefix_len) override;
    IPAddress::Ptr mask(uint32_t prefix_len) override;
    uint32_t port() const override;
    void set_port(uint16_t v) override;
private:
    sockaddr_in m_addr;
};

/**
 * @brief UnixSocket地址
 */
class UnixAddress : public Address {
public:
    typedef std::shared_ptr<UnixAddress> ptr;

    /**
     * @brief 无参构造函数
     */
    UnixAddress();

    /**
     * @brief 通过路径构造UnixAddress
     * @param[in] path UnixSocket路径(长度小于UNIX_PATH_MAX)
     */
    UnixAddress(const std::string& path);

    const sockaddr* get_addr() const override;
    sockaddr* get_addr() override;
    socklen_t size() const override;
    void set_size(uint32_t v);
    std::string get_path() const;
    std::ostream& insert(std::ostream& os) const override;
private:
    sockaddr_un m_addr;
    socklen_t m_length;
};

FISH_NAMESPACE_END
