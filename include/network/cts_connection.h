#pragma once

#include "network/connection.h"
#include "network/client.h"

/** Concrete implementation of a Client-to-Server
 * worker which handles communications between a client
 * node and the server. */
class CtSConnection : public Connection {
protected:
    /** A reference to the client which this connection worker is representing. */
    Client& _client;

    /** Overriden parse_data method which correctly parses Client-To-Server
     * specific packets. */
    ParseResult _parse_data(Packet &packet) override;

public:

    /** Constructor that takes a socket, the client it represents, and the
     * address of the server to connect to. */
    CtSConnection(int fd, Client& c, sockaddr_in server);

    /** Overriden run method which implements the main even loop. In this case,
     * it initially connects to the server and registers this client. It then updates
     * the local client about other clients on the network whenever it receives
     * a new list from the server. At the end, it deregisters this client from the server.
     */
    void run() override;

    /** Sends a packet containing the address of the local client to server so
     * it can inform other remote clients that this one exists and can be
     * connected to. */
    void register_with_server();

    /** Tells the server this client is shutting down, and so to inform other clients
     * it is no longer available on the network. */
    void deregister_and_shutdown();

};
