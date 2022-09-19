#include <memory>
#include <sstream>
#include <netdb.h>
#include <ifaddrs.h>
#include <stddef.h>
#include "address.h"

#include "common.h"
#include "endian.h"

FISH_NAMESPACE_START

template<class T>
static T createMask(uint32_t bits) {
    return (1 << (sizeof(T) * 8 - bits)) - 1;
}

template<class T>
static uint32_t CountBytes(T value) {
    uint32_t result = 0;
    for(; value; ++result) {
        value &= value - 1;
    }
    return result;
}

Address::Ptr Address::lookup_any(const std::string& host, int family, int type, int protocol) {
    std::vector<Address::Ptr> result;
    if(lookup(result, host, family, type, protocol)) {
        return result[0];
    }
    return nullptr;
}

IPAddress::Ptr Address::lookup_any_ipaddress(const std::string& host,
                                int family, int type, int protocol) {
    std::vector<Address::Ptr> result;
    if(lookup(result, host, family, type, protocol)) {
        //for(auto& i : result) {
        //    std::cout << i->to_string() << std::endl;
        //}
        for(auto& i : result) {
            IPAddress::Ptr v = std::dynamic_pointer_cast<IPAddress>(i);
            if(v) {
                return v;
            }
        }
    }
    return nullptr;
}


bool Address::lookup(std::vector<Address::Ptr>& result, const std::string& host,
                     int family, int type, int protocol) {
    addrinfo hints, *results, *next;
    hints.ai_flags = 0;
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    hints.ai_addrlen = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    std::string node;
    const char* service = NULL;

    //检查 node serivce
    if(node.empty()) {
        service = (const char*)memchr(host.c_str(), ':', host.size());
        if(service) {
            if(!memchr(service + 1, ':', host.c_str() + host.size() - service - 1)) {
                node = host.substr(0, service - host.c_str());
                ++service;
            }
        }
    }

    if(node.empty()) {
        node = host;
    }
    int error = getaddrinfo(node.c_str(), service, &hints, &results);
    if(error) {
        FISH_LOGERROR("Address::lookup getaddress(" << host << ", "
            << family << ", " << type << ") err=" << error << " errstr="
            << gai_strerror(error));
        return false;
    }

    next = results;
    while(next) {
        result.push_back(create(*next->ai_addr));
        next = next->ai_next;
    }

    freeaddrinfo(results);
    return !result.empty();
}

bool Address::get_interface_addresses(std::multimap<std::string
                    ,std::pair<Address::Ptr, uint32_t> >& result,
                    int family) {
    struct ifaddrs *next, *results;
    if(getifaddrs(&results) != 0) {
        FISH_LOGERROR("Address::get_interface_addresses getifaddrs"
            << " err=" << errno << " errstr=" << strerror(errno));
        return false;
    }

    try {
        for(next = results; next; next = next->ifa_next) {
            Address::Ptr addr;
            uint32_t prefix_len = ~0u;
            if(family != AF_UNSPEC && family != next->ifa_addr->sa_family) {
                continue;
            }
            switch(next->ifa_addr->sa_family) {
                case AF_INET:
                    {
                        addr = create(*next->ifa_addr);
                        uint32_t netmask = ((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;
                        prefix_len = CountBytes(netmask);
                    }
                    break;
                case AF_INET6:
                    {
                        addr = create(*next->ifa_addr);
                        in6_addr& netmask = ((sockaddr_in6*)next->ifa_netmask)->sin6_addr;
                        prefix_len = 0;
                        for(int i = 0; i < 16; ++i) {
                            prefix_len += CountBytes(netmask.s6_addr[i]);
                        }
                    }
                    break;
                default:
                    break;
            }

            if(addr) {
                result.insert(std::make_pair(next->ifa_name,
                            std::make_pair(addr, prefix_len)));
            }
        }
    } catch (...) {
        FISH_LOGERROR("Address::get_interface_addresses exception");
        freeifaddrs(results);
        return false;
    }
    freeifaddrs(results);
    return !result.empty();
}

bool Address::get_interface_addresses(std::vector<std::pair<Address::Ptr, uint32_t> >&result ,const std::string& iface, int family) {
    if(iface.empty() || iface == "*") {
        if(family == AF_INET || family == AF_UNSPEC) {
            result.push_back(std::make_pair(Address::Ptr(new IPv4Address()), 0u));
        }
        else return false;
        return true;
    }
    std::multimap<std::string ,std::pair<Address::Ptr, uint32_t> > results;
    if(!get_interface_addresses(results, family)) {
        return false;
    }

    auto its = results.equal_range(iface);
    for(; its.first != its.second; ++its.first) {
        result.push_back(its.first->second);
    }
    return !result.empty();
}

int Address::get_family() const {
    return get_addr()->sa_family;
}

std::string Address::to_string() const {
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

Address::Ptr Address::create(const sockaddr &addr) {
    switch(addr.sa_family) {
        case AF_INET:
            return std::make_shared<IPv4Address>(reinterpret_cast<const sockaddr_in&>(addr));
    }
    return nullptr;
}

bool Address::operator<(const Address& rhs) const {
    socklen_t minlen = std::min(size(), rhs.size());
    int result = memcmp(get_addr(), rhs.get_addr(), minlen);
    if(result < 0) {
        return true;
    } else if(result > 0) {
        return false;
    } else if(size() < rhs.size()) {
        return true;
    }
    return false;
}

bool Address::operator==(const Address& rhs) const {
    return size() == rhs.size()
        && memcmp(get_addr(), rhs.get_addr(), size()) == 0;
}

bool Address::operator!=(const Address& rhs) const {
    return !(*this == rhs);
}

IPAddress::Ptr IPAddress::create(const char* address, uint16_t port) {
    addrinfo hints, *results;
    memset(&hints, 0, sizeof(addrinfo));

    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;

    int error = getaddrinfo(address, NULL, &hints, &results);
    if(error) {
        FISH_LOGERROR("IPAddress::create(" << address
            << ", " << port << ") error=" << error
            << " errno=" << errno << " errstr=" << strerror(errno));
        return nullptr;
    }

    try {
        IPAddress::Ptr result = std::dynamic_pointer_cast<IPAddress>(Address::create(*results->ai_addr));
        if(result) {
            result->set_port(port);
        }
        freeaddrinfo(results);
        return result;
    } catch (...) {
        freeaddrinfo(results);
        return nullptr;
    }
}

IPv4Address::Ptr IPv4Address::create(const char* address, uint16_t port) {
    auto rt = std::make_shared<IPv4Address>();
    rt->m_addr.sin_port = ByteSwapOnLittleEndian(port);
    int result = inet_pton(AF_INET, address, &rt->m_addr.sin_addr);
    if(result <= 0) {
        FISH_LOGERROR("IPv4Address::create(" << address << ", "
                << port << ") rt=" << result << " errno=" << errno
                << " errstr=" << strerror(errno));
        return nullptr;
    }
    return rt;
}

IPv4Address::IPv4Address(const sockaddr_in& address) {
    m_addr = address;
}

IPv4Address::IPv4Address(uint32_t address, uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = ByteSwapOnLittleEndian(port);
    m_addr.sin_addr.s_addr = ByteSwapOnLittleEndian(address);
}

sockaddr* IPv4Address::get_addr() {
    return (sockaddr*)&m_addr;
}

const sockaddr* IPv4Address::get_addr() const {
    return (sockaddr*)&m_addr;
}

socklen_t IPv4Address::size() const {
    return sizeof(m_addr);
}

std::ostream& IPv4Address::insert(std::ostream& os) const {
    uint32_t addr = ByteSwapOnLittleEndian(m_addr.sin_addr.s_addr);
    os << ((addr >> 24) & 0xff) << "."
       << ((addr >> 16) & 0xff) << "."
       << ((addr >> 8) & 0xff) << "."
       << (addr & 0xff);
    os << ":" << ByteSwapOnLittleEndian(m_addr.sin_port);
    return os;
}

IPAddress::Ptr IPv4Address::broadcast(uint32_t prefix_len) {
    if(prefix_len > 32) {
        return nullptr;
    }

    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr |= ByteSwapOnLittleEndian(
            createMask<uint32_t>(prefix_len));
    return IPv4Address::Ptr(new IPv4Address(baddr));
}

IPAddress::Ptr IPv4Address::network(uint32_t prefix_len) {
    if(prefix_len > 32) {
        return nullptr;
    }

    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr &= ByteSwapOnLittleEndian(
            createMask<uint32_t>(prefix_len));
    return IPv4Address::Ptr(new IPv4Address(baddr));
}

IPAddress::Ptr IPv4Address::mask(uint32_t prefix_len) {
    sockaddr_in subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin_family = AF_INET;
    subnet.sin_addr.s_addr = ~ByteSwapOnLittleEndian(createMask<uint32_t>(prefix_len));
    return IPv4Address::Ptr(new IPv4Address(subnet));
}

uint32_t IPv4Address::port() const {
    return ByteSwapOnLittleEndian(m_addr.sin_port);
}

void IPv4Address::set_port(uint16_t v) {
    m_addr.sin_port = ByteSwapOnLittleEndian(v);
}

static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0)->sun_path) - 1;

UnixAddress::UnixAddress() {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

UnixAddress::UnixAddress(const std::string& path) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = path.size() + 1;

    if(!path.empty() && path[0] == '\0') {
        --m_length;
    }

    if(m_length > sizeof(m_addr.sun_path)) {
        throw std::logic_error("path too long");
    }
    memcpy(m_addr.sun_path, path.c_str(), m_length);
    m_length += offsetof(sockaddr_un, sun_path);
}

void UnixAddress::set_size(uint32_t v) {
    m_length = v;
}

sockaddr* UnixAddress::get_addr() {
    return (sockaddr*)&m_addr;
}

const sockaddr* UnixAddress::get_addr() const {
    return (sockaddr*)&m_addr;
}

socklen_t UnixAddress::size() const {
    return m_length;
}

std::string UnixAddress::get_path() const {
    std::stringstream ss;
    if(m_length > offsetof(sockaddr_un, sun_path)
            && m_addr.sun_path[0] == '\0') {
        ss << "\\0" << std::string(m_addr.sun_path + 1,
                m_length - offsetof(sockaddr_un, sun_path) - 1);
    } else {
        ss << m_addr.sun_path;
    }
    return ss.str();
}

std::ostream& UnixAddress::insert(std::ostream& os) const {
    if(m_length > offsetof(sockaddr_un, sun_path)
            && m_addr.sun_path[0] == '\0') {
        return os << "\\0" << std::string(m_addr.sun_path + 1,
                m_length - offsetof(sockaddr_un, sun_path) - 1);
    }
    return os << m_addr.sun_path;
}

std::ostream& operator<<(std::ostream& os, const Address& addr) {
    return addr.insert(os);
}

FISH_NAMESPACE_END
