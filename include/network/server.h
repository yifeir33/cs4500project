#pragma once

#include <mutex>
#include <atomic>

#include "network/netport.h"
#include "network/packet.h"

/** Concrete implementation of a NetPort representing a server. 
 * Desiugned to hold the list of clients on the network, allowing new clients
 * to register and to inform all client of what other clients are on the network. */
class Server : public NetPort {
protected:
    /** The mutex protecting the list of registered clients. */
    std::mutex _client_mutex;
    /** The list of registerd clients. */
    std::vector<sockaddr_in> _clients;
    /** The amount of connection workers which have passed on the latest update
     * of the number of clients to their resepective end nodes. */
    std::atomic<size_t> _passed_update;
    /** The amount of connection workers expected to have passed on the latest update
     * of the number of clients to their resepective end ndoes. */
    std::atomic<size_t> _expected_update;
    /** True if there is a new update of clients on the network to be passed on. */
    std::atomic<bool> _new_update;

    /** Private constructor for the NetPort singleton pattern. */
    Server(const char *ip, in_port_t port);

    /** Delete copy/move semantics. */
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server(Server&&) = delete;

    /** Implementation of the intial set-up down by the server before entering
     * the main event loop. In this implementation does nothing. */
    void _initial() override;

    /** Implementation of the main event loop work to be done by the server.
     * In this implementation, does nothing. */
    void _main_loop_work() override;

    /** Implementation of the work to be done by the server when a new connection
     * is accepted. In this implementation, does nothing. */
    void _on_new_connection() override;

    /** Implementation of the work to be done by the server when a finished connection
     * worker is cleaned up. In this implementation, removes its client from the
     * list of active clients on the network. */
    void _on_clean_up(std::shared_ptr<Connection> c) override;

    /** Implementation to spawn the correct connection worker for connections accepted
     * by the server. In this implementation spawns a ServerConnection worker. */
    std::shared_ptr<Connection> _new_connection(int new_conn_fd, sockaddr_in other) override;

public:
    /** Static method to intialize the NetPort instance pointer to be a server.
     * Returns true if it succeeds, false otherwise. */
    static bool init(const char *ip, in_port_t port);
    /** Returns a pointer to the single NetPort instance as a Server. If it is
     * not a server, a nullptr is returned. */
    static std::weak_ptr<Server> get_instance();

    /** Updates the list of clients with the given address, and sets up the local
     * variables to ensure that the server connections pass the update onto their
     * remote nodes. */
    void update_and_alert(sockaddr_in caddr);

    /** Returns a packet containing the list of clients to be sent to remote nodes. */
    Packet get_clients();

    /** Removes the given client from the list of active clients, and sets up
     * the local variables to ensure that the server connections pass the update
     * onto their remote nodes. */
    void remove_client(sockaddr_in client);

    /** Returns true if there is an update to the list of clients that has not
     * yet been passed on to all nodes. */
    bool new_client_update();

    /** Used to request a network teardown. In this case instructs all clients to
     * shutdown, before shutting down the server itself. */
    void request_teardown() override;

    /** The ServerConnection class is the concrete implementation of the connection
     * worker used to communicate with clients which have connected to this server.
     * As it can be considered an extension of the server itself, it is a friend
     * class. */
    friend class ServerConnection;
};
