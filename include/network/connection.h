#pragma once

#include <chrono>
#include <thread>
#include <atomic>
#include <netinet/ip.h>
#include <mutex>

#include "util/object.h"
#include "network/packet.h"

#define WATCHDOG_TIMEOUT  60 // seconds
#define BUFFER_SIZE       4096 // half-page buffer

/** Enum representing the result of parsing a packet. */
enum ParseResult {
    /** An error occured while parsing the packet. */
    ParseError = -1,
    /** The packet was parsed sucessfully, and may be dropped. */
    Success = 0,
    /** The packet was parsed sucessfully, but requires a request
     * to complete the transaction. */
    Response = 1,
};


/** Abstract worker class. This represents the abstract version of a worker
 * which handles communication across the network over the TCP connection
 * associated with the given socket. */
class Connection : public Object {
private:
    /** The thread this worker runs its even loop on. */
    std::thread _thread;
    /** The mutex protecting the socket file descriptor. */
    std::mutex _fd_mutex;
    /** The TCP socket file descriptor. */
    int _conn_fd;

    /** Delete copy constructors as it doesn't make sense for this type
     * of object. */
    Connection(const Connection& c) = delete;
    Connection& operator=(const Connection& c) = delete;

protected:
    /** The address of the node this worker is connected to. */
    sockaddr_in _conn_other;
    /** An atomic boolean which is false when this is still working, but
     * is set to true to either cause this to finish its work, and to signify
     * that this is done with its work. */
    std::atomic<bool> _finished;
    /** A value representing the end of the data in the receive buffer. */
    size_t _r_buf_pos;
    /** The receive buffer used by this socket to receive data. */
    uint8_t _r_buffer[BUFFER_SIZE];
    /** A timepoint that is used to track when the last communication over the
     * connection was. */
    std::chrono::time_point<std::chrono::steady_clock> _watchdog;

    /** Sends a keep-alive packet to the other endpoint of the connection
     * to update the stored timepoint on both ends. */
    bool _keep_alive();

    /** Sends a packet signifying that this worker is shutting down to the 
     * other end of the connection. */
    void _send_shutdown();

    /** Sends the given packet over the connection. */
    bool _send_packet(Packet& packet);

    /** Checks to see if there are any errors associated with the socket file
     * descriptor. Returns the number of errors. */
    int _check_for_socket_errors();

    /** A method that parses the data received over the network. */
    virtual ParseResult _parse_data(Packet& packet);

    /** Used if ParseResult::Respond is returned from _parse_data, signifying
     * that a response is required to that packet. Takes the packet and constructs
     * and sends the response. */
    virtual int _respond(Packet& msg);

    /** Pure virtual method that is used to implement the main event loop for
     * the concrete worker implementation. */
    virtual void run() = 0;

public:
    /** Constructor initializing the internals of this class. */
    Connection(int cfd, sockaddr_in c);

    /** Virtual destructor which closes the socket. */
    virtual ~Connection();

    /** Starts the worker logic that run provides in its own thread. */
    void start();

    /** Joins the thread the worker is running on. */
    void join();

    /** Updates the _watchdog timepoint to the current time. */
    void feed_dog();

    /** Returns true if the time between now and the last time the _watchdog
     * timepoint was updated is less than the WATCHDOG_TIMEOUT in seconds,
     * false otherwise. */
    bool dog_is_alive() const;

    /** Returns the address of the node that this connection is communicating 
     * with. */
    sockaddr_in get_conn_other() const;

    /** Receives data from the socket (if there is data to be received). If
     * there is no error, returns the number of bytes received. Otherwise,
     * returns -1. */
    int receive_data();

    /** Reads the data from the receive buffer into the packet at the given
     * reference, so that it is easier to parse. */
    bool unpack(Packet& packet);

    /** Receives data from the socket, unpacks it into a packet, and 
     * then parses the data. If a response is returned  by the parse function,
     * this also calls the _respond function. Returns true if there were no
     * errors at any point, false otherwise. */
    bool receive_and_parse();

    /** Connects to the node at the given target address over the internal TCP
     * socket. */
    void connect_to_target(sockaddr_in target);

    /** Asks this worker to stop working and close the connection. It does this
     * by setting the _finshed boolean to true. */
    void ask_to_finish();

    /** Sends a packet asking for a teardown of the network. */
    void send_teardown_request();

    /** Returns if this is finished with its work or not. */
    bool is_finished();

    /** Prints the data in the read_buffer for debugging purposes. */
    void print_read_buffer();

    /** Implementation of clone for the Object superclass. Returns nullptr,
     * as this class cannot be cloned. */
    std::shared_ptr<Object> clone() const override;

    /** Tests for pointer equality. */
    bool equals(const Object* other) const override;

    /** Returns the hashcode of this object. */
    size_t hash() const override;
};
