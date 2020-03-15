#pragma once
#include <netinet/ip.h>

namespace socket_util {
    inline bool get_socket_info(int fd, sockaddr_in& saddr);
    inline bool make_non_blocking(int fd);
    inline int create_socket(sockaddr_in& self, bool non_block);
    inline int create_socket(const char *ip, in_port_t port, sockaddr_in& self, bool non_block);
    inline void clone_sockaddr(sockaddr_in& dest, const sockaddr_in& from);
    inline bool sockaddr_eq(const sockaddr_in& one, const sockaddr_in& two);
};