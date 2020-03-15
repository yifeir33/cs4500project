#pragma once

#include <atomic>
#include <netinet/ip.h>

#include "thread.h"
#include "packet.h"

#define WATCHDOG_TIMEOUT  60 // seconds
#define BUFFER_SIZE (2 * PACKET_MAX_SIZE)

class Connection : public Thread {
protected:
    int _conn_fd;
    sockaddr_in _conn_other;
    std::atomic<bool> _finished;
    size_t _r_buf_pos;
    uint8_t _r_buffer[BUFFER_SIZE];
    std::chrono::time_point<std::chrono::steady_clock> _watchdog;

    bool _keep_alive();

    void _send_shutdown();

    bool _send_packet(Packet *packet);

    int _check_for_socket_errors();

    virtual ParseResult _parse_data(Packet& packet);

    virtual int _respond(Packet& msg);

public:

    Connection(int cfd, sockaddr_in c);

    virtual ~Connection();

    void feed_dog();

    bool dog_is_alive() const;

    sockaddr_in get_conn_other() const;

    int receive_data();

    bool unpack(Packet& packet);

    bool receive_and_parse();

    void connect_to_target(sockaddr_in target);

    void ask_to_finish();

    bool is_finished();

    void print_read_buffer();
};
