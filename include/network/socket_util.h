#pragma once
#include <netinet/ip.h>

/** Namespace containing some convience functions for creating and manipulating
 * sockets. */
namespace socket_util {
    /** Given a socket, fills the given sockaddr_in with the address and
     * port the socket is bound to. Returns true if it suceeds, false otherwise. */
    bool get_socket_info(int fd, sockaddr_in& saddr);
    /** Given a socket, attempts to make it non-blocking. Returns true if it suceeds,
     * false otherwise. */
    bool make_non_blocking(int fd);
    /** Given an address, and whether or not the socket should be non-blocking,
     * creates a new socket bound to the given address and returns it. Returns
     * -1 on errors. */
    int create_socket(sockaddr_in& self, bool non_block);
    /** Given an ip as a cstring, a port, an object to store the socket address
     * in, and whether or not to make the socket non blocking, converts the 
     * ip address and port to the correct format, stores them in the given 
     * sockaddr_in object, and creates a new socket bound to that address and
     * returns it. Returns -1 on errors. */
    int create_socket(const char *ip, in_port_t port, sockaddr_in& self, bool non_block);
    /** Given a sockaddr_in to copy the data into, and a sockaddr_in to copy
     * the data from, copies the data from one sockaddr_in to the other. */
    void clone_sockaddr(sockaddr_in& dest, const sockaddr_in& from);
    /** Compares two sockaddr_ins to see if they represent the same
     * address. Returns true if they are equal, false otherwise. */
    bool sockaddr_eq(const sockaddr_in& one, const sockaddr_in& two);
}
