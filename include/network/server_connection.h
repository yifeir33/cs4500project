#pragma once

#include "network/connection.h"
#include "network/server.h"


/** Concrete implemntation of a connection worker used to communicate from
 * the Server instance to connected clients. */
class ServerConnection : public Connection {
protected:
    /** A reference to the server this connection worker represents. */
    Server& _server;

    /** Overriden parse_data method to provide the correct parsing for
     * Server-to-Client communication specific packets. */
    ParseResult _parse_data(Packet& packet) override;

public:
    /** Constructor to create a new connection worker, giving the socket
     * which is connected to a client, the address of the client (as noted 
     * by the connection), and the reference to the server it represents. */
    ServerConnection(int cfd, sockaddr_in caddr, Server& s);

    /** Override of the run method which implements the main event loop. In this
     * implemntation it simply checks for communication from the client and 
     * pares it, and passes updates to the client list to the client. */
    void run() override;
};
