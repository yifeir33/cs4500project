#pragma once

#include "connection.h"
#include "server.h"


class ServerConnection : public Connection {
public:
    Server& _server;
    SockAddrWrapper *_client;

    ServerConnection(int cfd, SockAddrWrapper *c, Server& s);

    ~ServerConnection();

    void run() override;

    ParseResult _parse_data(Packet& packet) override;
};
