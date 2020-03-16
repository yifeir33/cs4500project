#pragma once

#include "connection.h"
#include "server.h"


class ServerConnection : public Connection {
protected:
    Server& _server;
    sockaddr_in _client;

    ParseResult _parse_data(Packet& packet) override;

public:
    ServerConnection(int cfd, sockaddr_in caddr, Server& s);

    virtual ~ServerConnection();

    void run() override;
};
