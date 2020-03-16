#pragma once
#include <netinet/ip.h>

namespace socket_util {
    bool get_socket_info(int fd, sockaddr_in& saddr);
    bool make_non_blocking(int fd);
    int create_socket(sockaddr_in& self, bool non_block);
    int create_socket(const char *ip, in_port_t port, sockaddr_in& self, bool non_block);
    void clone_sockaddr(sockaddr_in& dest, const sockaddr_in& from);
    bool sockaddr_eq(const sockaddr_in& one, const sockaddr_in& two);
};
