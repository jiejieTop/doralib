/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2019-12-09 21:31:25
 * @LastEditTime: 2020-10-29 16:32:39
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#ifndef DORA_SOCKET_H
#define DORA_SOCKET_H

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <map>

#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#include "dora_exception.h"
#include "dora_nocopyable.h"

using namespace std;

namespace doralib 
{

#define DORA_ADDR_ANY ((const char *)"0.0.0.0")

/**
 * @brief sock is the base class of every other doralib object.
 *
 * It provides the most important field: The sfd field holds the file descriptor
 * for every sock.
 */
typedef enum sock_type {
    DORA_SOCK_TCP = SOCK_STREAM,
    DORA_SOCK_UDP = SOCK_DGRAM,
    DORA_SOCK_RAW = SOCK_RAW,
    DORA_SOCK_RDM = SOCK_RDM,
    DORA_SOCK_SEQPACKET = SOCK_SEQPACKET,
    DORA_SOCK_DCCP = SOCK_DCCP
} sock_type_t;

typedef enum sock_family {
    /// IPv4 socket
    DORA_SOCK_IPv4 = AF_INET,
    /// IPv6 socket
    DORA_SOCK_IPv6 = AF_INET6,
    /// Unix socket
    DORA_SOCK_UNIX = AF_UNIX,
} sock_family_t;

class sock : public noncopyable
{
private:
    /// fd is the sockets API file descriptor
    int _sockfd;
    int _type;
    int _family;
    int _protocol;

    bool _is_connected;
    bool _is_nonblocking;
    /// Default is true; if set to false, the file descriptor is not closed when
    /// the destructor is called.
    bool _close_on_destructor;

public:
    sock();
    sock(int _type, int _family, int _protocol);
    sock(const sock&) = delete;
    sock(sock&&);

    ~sock();
    
    void sock_init(void);
    int sock_socket(void);

    int sock_close(void);

    int sock_bind(const sockaddr_in *addr);
    int sock_bind(const sockaddr_in & addr);
    int sock_bind(const char *addr, const char *port);
    int sock_connect(const sockaddr_in *addr);
    int sock_connect(const sockaddr_in & addr);
    int sock_connect(const char *addr, const char *port);
    int sock_connect(const std::string & addr, const std::string & port);


    ssize_t sock_read(void *buf, size_t len, int timeout = -1, int flags = 0);
    ssize_t sock_read(void *buf, size_t len, struct sockaddr *src_addr, socklen_t *addrlen, int timeout = -1, int flags = 0);
    
    ssize_t sock_write(const void *buf, size_t len, int timeout = -1);
    ssize_t sock_write(const void *buf, size_t len, struct sockaddr *dest_addr, socklen_t addrlen, int timeout = -1, int flags = 0);

    bool sock_is_connected(void) { return _is_connected; }
    bool sock_is_nonblocking(void) { return _is_nonblocking; }
    int sock_get_sockfd(void) { return _sockfd; }
    int sock_get_family(void) { return _family; }
    int sock_get_type(void) { return _type; }
    int sock_get_protocol(void) { return _protocol; }

    int sock_set_opt(int level, int optname, const char* optval, socklen_t optlen) const;
    void sock_set_timeout(int v);

    void sock_set_cod(bool cod) { _close_on_destructor = cod; }
    int sock_set_block(void);
    int sock_set_nonblock(void);

};

}  // namespace doralib

#endif
