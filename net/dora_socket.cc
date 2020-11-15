/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2019-12-09 21:31:25
 * @LastEditTime: 2020-11-15 11:55:21
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#include "dora_socket.h"
#include "dora_error.h"
#include "dora_log.h"
#include "dora_exception.h"

namespace doralib {


int sock::sock_set_addr(sockaddr_in & sockaddr, const char *addr, const char *port)
{
    int ret = DORA_SOCKET_UNKNOWN_HOST;
    struct addrinfo hints, *addr_list, *cur;

    bzero(&sockaddr,sizeof(sockaddr));
    
    /* Do name resolution with both IPv6 and IPv4 */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = _type;
    hints.ai_protocol = IPPROTO_UDP;
    
    if (getaddrinfo(addr, port, &hints, &addr_list) != 0) {
        DORA_LOG_ERROR("can not get addr info ...");
        return ret;
    }

    if (getaddrinfo((const char *)addr, (const char *)port, &hints, &addr_list) != 0) {
        DORA_LOG_ERROR("can not get addr info ...");
        return ret;
    }

    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        switch (cur->ai_family) {
        case AF_UNSPEC:
            DORA_LOG_ERROR("unspecified");
            break;
        case AF_INET:
            struct sockaddr_in *addr;
            addr = (struct sockaddr_in *)cur->ai_addr;
            memcpy(&sockaddr, addr, sizeof(struct sockaddr_in));
            ret = DORA_SUCCESS;
            break;
        case AF_INET6:
            DORA_LOG_ERROR("not support!");
        }
    }

    freeaddrinfo(addr_list);

    sockaddr.sin_family = _family;
    sockaddr.sin_port = htons(atoi((const char *)port));

    return ret;
}

int sock::sock_set_addr(sockaddr_in & sockaddr, const std::string & addr, const std::string & port)
{
    int ret = DORA_SOCKET_UNKNOWN_HOST;
    struct addrinfo hints, *addr_list, *cur;

    bzero(&sockaddr,sizeof(sockaddr));
    
    /* Do name resolution with both IPv6 and IPv4 */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = _type;
    hints.ai_protocol = IPPROTO_UDP;
    
    if (getaddrinfo((const char *)addr.c_str(), (const char *)port.c_str(), &hints, &addr_list) != 0) {
        DORA_LOG_ERROR("can not get addr info ...");
        return ret;
    }

    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        switch (cur->ai_family) {
        case AF_UNSPEC:
            DORA_LOG_ERROR("unspecified");
            break;
        case AF_INET:
            struct sockaddr_in *addr;
            addr = (struct sockaddr_in *)cur->ai_addr;
            memcpy(&sockaddr, addr, sizeof(struct sockaddr_in));
            ret = DORA_SUCCESS;
            break;
        case AF_INET6:
            DORA_LOG_ERROR("not support!");
        }
    }

    freeaddrinfo(addr_list);

    sockaddr.sin_family = _family;
    sockaddr.sin_port = htons(atoi((const char *)port.c_str()));

    return ret;
}

/**
 * @brief Constructor. Sets `_sockfd` to -1.
 *
 */
sock::sock(int _type, int _family, int _protocol)
    : _sockfd(-1)
    , _type(_type)
    , _family(_family)
    , _protocol(_protocol)
    , _is_connected(false)
    , _is_nonblocking(false)
    , _close_on_destructor(true) 
{
    bzero(&_remote_addr,sizeof(_remote_addr));
    bzero(&_local_addr, sizeof(_local_addr));
    _sockfd = sock_socket();
}

sock::sock(const char *addr, const char *port, int _type, int _family, int _protocol)
    : _sockfd(-1)
    , _type(_type)
    , _family(_family)
    , _protocol(_protocol)
    , _is_connected(false)
    , _is_nonblocking(false)
    , _close_on_destructor(true) 
{
    bzero(&_remote_addr,sizeof(_remote_addr));
    bzero(&_local_addr, sizeof(_local_addr));
    _sockfd = sock_socket();
    this->sock_set_addr(_remote_addr, addr, port);
}
sock::sock(const std::string & addr, const std::string & port, int _type, int _family, int _protocol)
    : _sockfd(-1)
    , _type(_type)
    , _family(_family)
    , _protocol(_protocol)
    , _is_connected(false)
    , _is_nonblocking(false)
    , _close_on_destructor(true) 
{
    bzero(&_remote_addr,sizeof(_remote_addr));
    bzero(&_local_addr, sizeof(_local_addr));
    _sockfd = sock_socket();
    this->sock_set_addr(_remote_addr, addr, port);
}

int sock::sock_socket(void) 
{
    _sockfd = socket(_family, _type, _protocol);
    if (-1 == _sockfd) {
        DORA_EXCEPTION("new socket fail ...");
        return -1;
    }

    DORA_LOG_DEBUG("socket fd is {}", _sockfd);

    if (_is_nonblocking) {
        sock_set_nonblock();
    }

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

int sock::sock_bind(void)
{
    if( bind(_sockfd, (struct sockaddr*)&_local_addr, sizeof(struct sockaddr_in)) < 0  ) {
        DORA_EXCEPTION("bind fail!");
        return -1;
    }
    return _sockfd;
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

int sock::sock_bind(const char *port)
{
    return sock_bind(NULL, port);
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

int sock::sock_listen(int backlog) 
{
    int ret = DORA_FAILED;
    ret = sock_bind();
    if (_type == DORA_SOCK_UDP) {
        return ret;
    }
    
    return listen(_sockfd, backlog);
}

int sock::sock_listen(const sockaddr_in *addr, int backlog) 
{
    int ret = DORA_FAILED;
    ret = sock_bind(addr);
    if (_type == DORA_SOCK_UDP) {
        return ret;
    }
    
    return listen(_sockfd, backlog);
}

int sock::sock_listen(const char *addr, const char *port, int backlog) 
{
    int ret = DORA_FAILED;
    ret = sock_bind(addr, port);
    if (_type == DORA_SOCK_UDP) {
        return ret;
    }
    
    return listen(_sockfd, backlog);
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

int sock::sock_disconnect(void)
{
    if (0 > _sockfd) 
        return DORA_NOTHING_FOUND;

    if (DORA_SOCK_UDP == _type) {
        _is_connected = false;
        return DORA_SUCCESS;
    }

    close(_sockfd);
    return DORA_SUCCESS;
}

int sock::sock_accept(struct sockaddr_in remote_addr, socklen_t *addrlen)
{
    return accept(_sockfd, (struct sockaddr *)&remote_addr, addrlen);
}

sock* sock::sock_accept(void)
{
    sock* s = new sock(DORA_SOCK_TCP);
    s->_sockfd = accept(_sockfd, (struct sockaddr *)&s->_remote_addr, NULL);
    if (s->_sockfd < 0) {
        return NULL;
    }
    return s;
}

int sock::sock_accept(sock *s)
{
    s->_sockfd = accept(_sockfd, (struct sockaddr *)&s->_remote_addr, NULL);
    if (s->_sockfd < 0) {
        return DORA_SOCKET_ACCEPT_FAILED;
    }
    return DORA_SUCCESS;
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

ssize_t sock::sock_write(const void *buf, size_t len, const char *addr, const char *port, int timeout, int flags)
{
    sockaddr_in sockaddr;
    sock_set_addr(sockaddr, addr, port);

    return sock_write(buf, len, (struct sockaddr *)&sockaddr, sizeof(sockaddr_in), timeout, flags);
}

ssize_t sock::sock_write(const void *buf, size_t len, const std::string & addr, const std::string & port, int timeout, int flags)
{
    sockaddr_in sockaddr;
    sock_set_addr(sockaddr, addr, port);

    return sock_write(buf, len, (struct sockaddr *)&sockaddr, sizeof(sockaddr_in), timeout, flags); 
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
