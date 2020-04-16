#pragma once

#include <mutex>
#include <atomic>
#include <arpa/inet.h>
#include <vector>

#include "network/netport.h"
#include "network/packet.h"
#include "data/kvstore.h"

// forward declarations to avoid circular dependancies. 
class CtSConnection;
class CtCConnection;

/** Concrete implementation of a NetPort that represents a client node on the network.
 * This registers with the server, and handles accepting and creating connections to
 * and from other client nodes, and facilitating cross-node tranfers of values. */
class Client : public NetPort {
private:
    /** The address of the server to connect to. */
    sockaddr_in _server;
    /** The connection worker class that communicates with the server. */
    std::unique_ptr<CtSConnection> _server_connection;
    /** A mutex protecting the list of other clients on the network. */
    std::mutex _oclient_mutex;
    /** The list of other client addresses on the network. */
    std::vector<sockaddr_in> _other_clients;
    /** Atomic boolean signifying there has been a new update to the clients
     * on the network. Used to communicate between this class and its thread
     * and the _server_connection thread, which receives the update. */
    std::atomic<bool> _client_update; 

    /** Private constructor for singleton pattern. */
    Client(const char *client_ip, const char *server_ip, in_port_t server_port);

    /** Deleted copy/move constructors for singleton pattern. */
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    Client(Client&&) = delete;

    /** The implementation of the initial work that the client does before entering
     * the main even loop. In this case it connects to the server, creating the
     * CtSConnection which is stored at _server_connection. */
    void _initial() override;

    /** The work down by the client every loop of the main event loop. In this case,
     * it checks too see if the list of clients has been updated, and if it has
     * ensures it has a connection to every other client on the network. It then 
     * ensures it still has an active server connection thread, recreating it
     * if it has failed due to an error. */
    void _main_loop_work() override;

    /** The work done by the client when a connection thread is cleaned up. In this
     * case it does nothing. */
    void _on_clean_up(std::shared_ptr<Connection> c) override;

    /** The work done by the client when a connection thread is created. In this case
     * it does nothing. */
    void _on_new_connection() override;

    /** The helper function to create a new connection worker when a connection is accepted
     * by the listening socket. In this case, as the client, it will only be connected to
     * by other clients, so it will create a new Client-to-Client worker (CtCConnection)
     * to communicate with the other client. */
    std::shared_ptr<Connection> _new_connection(int new_conn_fd, sockaddr_in other) override;

    /** Function that helps to get the value associated with the given key stored
     * on the node that the given connection. This thread blocks while waiting for
     * a response with the value, so it should never be called on a thread
     * handling network operations. It is called from the public get_value() method,
     * which should be called by external threads.
     *
     * WARNING: Blocking */
    std::shared_ptr<DataFrame> _get_value_helper(std::shared_ptr<CtCConnection> c, const std::string& key);

    /** Given an address, connects to the client running at that address 
     * using a Client-to-Client worker (CtCConnection) and add it to the list
     * of active connections. */
    void _connect_to_client(sockaddr_in client);

    /** Checks to see if the list of other clients on the network has been updated. 
     * If it has, it opens a connection to any client which it does not already have
     * and active connection with. */
    void _check_client_updates();

public:
    /** The netport uses a singleton pattern, but can only have one of EITHER a 
     * Server or Client. This method initializes the NetPort static variable to be
     * a Client instance using the private constructor. Returns true if the client
     * instance is created, false otherwise.
     */
    static bool init(const char *client_ip, const char *server_ip, in_port_t server_port);

    /** Returns a reference to the singleton pattern Client. If it is not a client,
     * returns a nullptr. */
    static std::weak_ptr<Client> get_instance();

    /** Destructor - closes server connection if it exists. */
    ~Client();

    /** Method that provides the packet to register with the server, contating this
     * client's addres. */
    Packet get_registration_packet();

    /** Method to externally ask the server to start a teardown of the entire
     * network. */
    void request_teardown() override;

    /** Method used by the KVStore to get the value associated with the given
     * key, which is non-local. This method will block until it receives a response
     * once it asks a remote node, and so should not be called by a thread using
     * networking. 
     *
     * WARNING: Blocking
     */
    std::shared_ptr<DataFrame> get_value(const KVStore::Key& key);

    /** Friend classes. These are workers which handle the communication over
     * a seperate socket from this node to other nodes or the server. Due to this,
     * they can be viewed as extensions of this class with specific functionality,
     * and so are considered friends. */
    friend class CtSConnection;
    friend class CtCConnection;
};
