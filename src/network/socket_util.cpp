#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "socket_util.h"

namespace socket_util{
    bool get_socket_info(int fd, sockaddr_in& saddr){
        socklen_t size = sizeof(saddr);
        if(getsockname(fd, reinterpret_cast<sockaddr *>(&saddr), &size) < 0){
            perror("Error getting socket info: ");
            return false;
        }
        assert(size == sizeof(saddr));
        return true;
    }

    bool make_non_blocking(int fd){
        // set socket to non-blocking
        if(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0){
            perror("Failed to set socket to non-blocking!");
            return false;
        }
        return true;
    }

    int create_socket(sockaddr_in& self, bool non_block){
        int sock_fd;
        // create socket
        if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Failed to create socket!");
            return -1;
        }

        if(non_block){
            if(!make_non_blocking(sock_fd)){
                std::cerr <<"Failed to make socket non-blocking!" <<std::endl;
                return -1;
            }
        }

        // let multiple sockets use same ip
        int opt = 1;
        if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
            perror("Error setting socket options: ");
            return -1;
        }
        // bind to address
        if(bind(sock_fd, reinterpret_cast<sockaddr *>(&self), sizeof(self)) < 0){
            perror("Failed to bind to address!");
            return -1;
        }


        return sock_fd;
    }

    int create_socket(const char *ip, in_port_t port, sockaddr_in& self, bool non_block){
        self.sin_family = AF_INET;
        if(ip){
            if(inet_pton(AF_INET, ip, &(self.sin_addr)) <= 0){ // convert to proper format
                perror("Error converting address: ");
                return -1;
            }
        } else {
            self.sin_addr.s_addr = INADDR_ANY;
        }
        self.sin_port = htons(port);

        int sock_fd = create_socket(self, non_block);

        // if we let the operating system set the port, get the info on where it was bound
        if(!ip || port == 0){
            if(!get_socket_info(sock_fd, self)){
                std::cerr <<"Failed to get socket info!" <<std::endl;
                return -1;
            }
        }

        return sock_fd;
    }

    void clone_sockaddr(sockaddr_in& dest, const sockaddr_in& from) {
        dest.sin_family = from.sin_family;
        dest.sin_port = from.sin_port;
        dest.sin_addr.s_addr = from.sin_addr.s_addr;
    }

    bool sockaddr_eq(const sockaddr_in& one, const sockaddr_in& two) {
        return one.sin_family == two.sin_family
            && one.sin_port == two.sin_port
            && one.sin_addr.s_addr == two.sin_addr.s_addr;
    }
}
