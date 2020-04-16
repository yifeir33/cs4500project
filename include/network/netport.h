#pragma once

#include <netinet/ip.h>
#include <chrono>
#include <vector>
#include <mutex>

#include "util/object.h"
#include "network/connection.h"

#define WATCHDOG_TIMEOUT  60 // seconds

/** Abstract class representing a object which listens for connections at
 * a given address. The parent of both Server and Client objects. Uses a modified
 * singleton pattern, where only one of its subclasses can exist locally. To do
 * this it provides a pointer to the NetPort instance, which should be
 * instantiated and accessed by subclasses to ensure only one NetPort 
 * class exists locally. */
class NetPort : public Object {
private:
    /** Mutex to protect the socket file descriptor. */
    std::mutex _fd_mutex;
    /** The file descriptor associated with the socket listening for
     * connections. */
    int _sock_fd;

    /** Delete copy and move semantics to enforce singleton. */
    NetPort(const NetPort&) = delete;
    NetPort& operator=(const NetPort&) = delete;
    NetPort(NetPort&&) = delete;

protected:
    /** Static mutex to protect the pointer of the netport from multiple classes
     * instantiating it at once. */
    static std::mutex instance_lock;
    /** Pointer to the single netport instance. Is null until initialized by
     * a subclass. */
    static std::shared_ptr<NetPort> np_instance;

    /** The address of the listening socket. */
    sockaddr_in _self;
    /** A mutex protecting the list of active connection workers that this
     * instance has spawned. */
    std::mutex _connections_mutex;
    /** A list of active connection workers spawned by this instance. */
    std::vector<std::shared_ptr<Connection>> _connections;
    /** Atomic boolean representing whether this object is still running or
     * not. Should only be false when the node is being shutdown. */
    std::atomic<bool> _running;
    /** Timepoint that represents the last time network traffic was received.
     * Used to ensure that work is being done by this node. */
    std::atomic<std::chrono::time_point<std::chrono::steady_clock>> _watchdog;

    /** Constructor to create a local socket. */
    NetPort(const char *ip, in_port_t port);

    /** Virtual destructor so that subclasses can provide their own destructors.
     * Also cleans up all spawned connection workers. */
    virtual ~NetPort();

    /** Updates the timepoint for the last time network traffic was received. */
    void feed_dog();

    /** Returns true if and only if the time elapsed between the last time
     * the _watchdog timepoint was updated and now is less than the WATCHDOG_TIMEOUT
     * preproccessor definition in seconds. */
    bool dog_is_alive() const;

    /** Closes all spawned connection workers and cleans them up. */
    void close_all();

    /** Accepts a new connection on the listening socket. */
    bool _accept_connection();

    /** Cleans up the connection workers signaling that they have finished. */
    void _clean_up_closed();

    /** Pure virtual function that is called when a connection is cleaned up
     * to allow subclasses to provide custom behavior. */
    virtual void _on_clean_up(std::shared_ptr<Connection> c) = 0;

    /** Pure virtual function that is called before the main event loop is entered
     * to allow subclasses to set up the local socket and connections. */
    virtual void _initial() = 0;

    /** Pure virtual function that is called every loop of the main even loop, and
     * allows workers to add their own functionality to the main event loop. */
    virtual void _main_loop_work() = 0;

    /** Pure virtual function that is called whenever a new connection is accepted,
     * to allow subclasses to provide custom behavior. */
    virtual void _on_new_connection() = 0;

    /** Pure virtual function so that workers can spawn the connection workers
     * that are relevant to their use case to handle an accepted connection. */
    virtual std::shared_ptr<Connection> _new_connection(int new_conn_fd, sockaddr_in other) = 0;

public:
    /** Starts to listen on the socket and run the main even loop,
     * with the given number of max concurrent connections. Blocks the
     * current thread indefinitely. 
     *
     * WARNING: Blocking */
    void listen_on_socket(int conn_count);

    /** Pure virtual function to allow requests to teardown the entire network,
     * with the behavior defined by the subclass. */
    virtual void request_teardown() = 0;

    /** Returns the hashcode of this object. */
    size_t hash() const override;

    /** Compares for pointer equality. */
    bool equals(const Object *other) const override;

    /** Implementation for Object.clone(), returns a nulptr
     * as this class cannot be cloned. */
    std::shared_ptr<Object> clone() const override;
};
