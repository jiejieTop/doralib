/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2019-12-09 21:31:25
 * @LastEditTime: 2020-11-02 09:59:04
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#include "dora_socket.h"
#include "dora_error.h"
#include "dora_exception.h"
#include "spdlog/spdlog.h"

namespace doralib {
/**
 * @brief Constructor. Sets `_sockfd` to -1.
 *
 */
sock::sock(void)
    :_sockfd(-1)
    ,_type(DORA_SOCK_TCP)
    ,_family(AF_INET)
    ,_protocol(0)
    ,_is_connected(false)
    ,_is_nonblocking(false)
    ,_close_on_destructor(true) {}

sock::sock(int _type, int _family = AF_INET, int _protocol = 0)
    :_sockfd(-1)
    ,_type(_type)
    ,_family(_family)
    ,_protocol(_protocol)
    ,_is_connected(false)
    ,_is_nonblocking(false)
    ,_close_on_destructor(true) {}

/**
 * @brief Move constructor.
 */
sock::sock(sock && other)
    :_sockfd(other._sockfd)
    ,_type(other._type)
    ,_family(other._family)
    ,_protocol(other._protocol)
    ,_is_connected(other._is_connected)
    ,_is_nonblocking(other._is_nonblocking)
    ,_close_on_destructor(true) 
{
    other._sockfd = -1;
}

void sock::sock_init(void) 
{
    if (_is_nonblocking) sock_set_nonblock();
    if (SOCK_DGRAM == _family) _is_connected = true;
}

int sock::sock_socket(void) 
{
    _sockfd = socket(_family, _type, _protocol);
    if (-1 != _sockfd) 
        sock_init();
    else
        DORA_EXCEPTION("new socket fail ...");
    DORA_LOG_INFO("socket fd is {}", _sockfd);
    return _sockfd;
}

/**
 * @brief Destructor: closes sock.
 */
sock::~sock(void) 
{
    /*
     * This is possible because sock::~sock(const sock&) is deleted.
     */
    if (_close_on_destructor) sock_close();
}

/**
 * @brief	Destroys a sock.
 *
 * @retval 0    Fine!
 * @retval <0   Most likely the sock was already closed before.
 */
int sock::sock_close(void) 
{
    if (0 > _sockfd) return 0;

    if (0 > close(_sockfd)) return -1;

    _sockfd = -1;

    return 0;
}

int sock::sock_bind(const sockaddr_in *addr)
{
    if( bind(_sockfd, (struct sockaddr*)addr, sizeof(struct sockaddr_in)) < 0  ) {
        DORA_EXCEPTION("bind fail!");
        return -1;
    }
    return _sockfd;
}

int sock::sock_bind(const sockaddr_in & addr)
{
    if( bind(_sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0  ) {
        DORA_EXCEPTION("bind fail!");
        return -1;
    }
    return _sockfd;
}

int sock::sock_bind(const char *addr, const char *port)
{
    struct sockaddr_in local;
    memset(&local, 0, sizeof(struct sockaddr_in));

    local.sin_family = AF_INET;
    local.sin_port = htons(atoi((const char*)port));
    
    if (NULL != addr) 
        local.sin_addr.s_addr = inet_addr(addr);
    else
        local.sin_addr.s_addr = htonl(INADDR_ANY);

    if( bind(_sockfd, (struct sockaddr*)&local, sizeof(local)) < 0  ) {
        DORA_EXCEPTION("bind fail!");
        return -1;
    }
    return _sockfd;
}


int sock::sock_connect(const sockaddr_in *addr)
{
    return connect(_sockfd, (const struct sockaddr *)addr, sizeof(struct sockaddr));
}

int sock::sock_connect(const sockaddr_in & addr)
{
    return connect(_sockfd, (const struct sockaddr *)&addr, sizeof(struct sockaddr));
}

int sock::sock_connect(const char *addr, const char *port)
{
    if (0 > _sockfd) 
        sock_socket();

    if (DORA_SOCK_UDP == _type) {
        _is_connected = true;
        return DORA_SUCCESS;
    }

    int ret = DORA_SOCKET_UNKNOWN_HOST;
    struct addrinfo hints, *addr_list, *cur;
    
    /* Do name resolution with both IPv6 and IPv4 */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = _type;
    hints.ai_protocol = (_type == SOCK_DGRAM) ? IPPROTO_UDP : IPPROTO_TCP;
    
    if (getaddrinfo(addr, port, &hints, &addr_list) != 0) {
        return ret;
    }
    
    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        if (connect(_sockfd, cur->ai_addr, cur->ai_addrlen) == 0) {
            ret = _sockfd;
            break;
        }
        close(_sockfd);
        DORA_LOG_ERROR("socket connect {} : {} fail ...", addr, port);
        ret = DORA_SOCKET_CONNECT_FAILED;
    }
    freeaddrinfo(addr_list);
    return ret;

}

int sock::sock_connect(const std::string & addr, const std::string & port)
{
    if (0 > _sockfd) 
        sock_socket();

    if (DORA_SOCK_UDP == _type) {
        _is_connected = true;
        return DORA_SUCCESS;
    }
    
    int ret = DORA_SOCKET_UNKNOWN_HOST;
    struct addrinfo hints, *addr_list, *cur;
    
    /* Do name resolution with both IPv6 and IPv4 */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = _type;
    hints.ai_protocol = (_type == SOCK_DGRAM) ? IPPROTO_UDP : IPPROTO_TCP;
    
    if (getaddrinfo((const char *)addr.c_str(), (const char *)port.c_str(), &hints, &addr_list) != 0) {
        DORA_LOG_ERROR("can not get addr info ...");
        return ret;
    }
    
    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        if (connect(_sockfd, cur->ai_addr, cur->ai_addrlen) == 0) {
            ret = _sockfd;
            break;
        }
        close(_sockfd);
        DORA_LOG_ERROR("socket connect {} : {} fail ...", addr, port);
        ret = DORA_SOCKET_CONNECT_FAILED;
    }
    freeaddrinfo(addr_list);
    return ret;
}

ssize_t sock::sock_read(void *buf, size_t len, int timeout, int flags)
{
    ssize_t rc = -1;
    rc = recv(_sockfd, buf, len, flags);
    if (0 == rc) {
        DORA_LOG_DEBUG("socket[{}] close", _sockfd);
        sock_close();
    }
    return rc;
}

ssize_t sock::sock_read(void *buf, size_t len, struct sockaddr *src_addr, socklen_t *addrlen, int timeout, int flags)
{
    sock_set_timeout(timeout);
    return recvfrom(_sockfd, buf, len, flags, src_addr, addrlen);
}

ssize_t sock::sock_write(const void *buf, size_t len, int timeout)
{
    return write(_sockfd, buf, len);
}

ssize_t sock::sock_write(const void *buf, size_t len, struct sockaddr *dest_addr, socklen_t addrlen, int timeout, int flags)
{
    sock_set_timeout(timeout);
    return sendto(_sockfd, buf, len, flags, dest_addr, addrlen);
}

/**
 * @brief Set sock options on the underlying sock.
 *
 * @return The return value of setsockopt(2).
 *
 * Sets sock options using setsockopt(2). See setsockopt(2), tcp(7), udp(7),
 * unix(7) for documentation on how to use this function.
 */
int sock::sock_set_opt(int level, int optname, const char* optval,
                         socklen_t optlen) const 
{
    return setsockopt(_sockfd, level, optname, optval, optlen);
}

int sock::sock_set_block(void)
{
    return fcntl(_sockfd, F_SETFL, fcntl(_sockfd, F_GETFL, F_GETFL) & ~O_NONBLOCK);
}

int sock::sock_set_nonblock(void)
{
    _is_nonblocking = true;
    return fcntl(_sockfd, F_SETFL, fcntl(_sockfd, F_GETFL, F_GETFL) | O_NONBLOCK);
}

void sock::sock_set_timeout(int v) 
{
    if ((_is_nonblocking) || (v <= 0))
        return;
    
    struct timeval tv {int(v / 1000), int(v % 1000 * 1000)};
    sock_set_opt(SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(struct timeval));
}

}  // namespace libsocket
