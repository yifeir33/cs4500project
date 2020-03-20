#pragma once

#include "network/client.h"
#include "network/connection.h"

class CtCConnection : public Connection {
protected:
    Client& _client;
    bool _receiver;

    void _as_client();

    void _as_receiver();

    ParseResult _parse_data(Packet& packet) override;

    int _respond(Packet& msg) override;

public:

    CtCConnection(int fd, sockaddr_in other, Client& c, bool r);

    void run() override;

    size_t hash() const override;
};
