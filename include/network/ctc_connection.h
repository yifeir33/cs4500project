#pragma once

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <list>

#include "network/client.h"
#include "network/connection.h"

class CtCConnection : public Connection {
public:
    class ValueRequest {
    public:
        std::string key;
        std::mutex mutex;
        std::condition_variable cv;
        std::shared_ptr<DataFrame> value;

        ValueRequest(std::string k);
    };

    CtCConnection(int fd, sockaddr_in other, Client& c, bool r);

    void add_request(std::shared_ptr<ValueRequest> request);

    void run() override;

    size_t hash() const override;

protected:
    Client& _client;
    bool _receiver;
    std::mutex _request_queue_mutex;
    std::queue<std::shared_ptr<ValueRequest>> _request_queue;
    std::mutex _waiting_requests_mutex;
    std::list<std::shared_ptr<ValueRequest>> _waiting_requests;

    void _as_client();

    void _as_receiver();

    void _send_keys();

    ParseResult _update_keys(Packet &packet);

    ParseResult _parse_data(Packet& packet) override;

    int _respond(Packet& msg) override;

    void _check_requests();

    ParseResult _parse_request_response(Packet &p);

    Packet _get_requested_value(std::string key);
};
