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
    class Request {
    public:
        virtual char get_type() const = 0;
    };

    class ValueRequest : public Request{
    public:
        std::string key;
        std::mutex mutex;
        std::condition_variable cv;
        std::shared_ptr<DataFrame> value;

        ValueRequest(std::string k);

        char get_type() const override;
    };

    class PushRequest : public Request {
    public:
        std::string dkey;
        std::vector<DistributedStore::MetaData> md;
        std::vector<std::string> lkeys;
        std::vector<std::shared_ptr<DataFrame>> ldfs;

        PushRequest(std::string k, std::vector<DistributedStore::MetaData> m,
                    std::vector<std::string> lks, std::vector<std::shared_ptr<DataFrame>> dfs);

        char get_type() const override;
    };

    class OperationRequest : public Request {
    public:
        std::string key;
        size_t opcode;
        std::mutex mutex;
        std::condition_variable cv;
        std::vector<uint8_t> result;

        OperationRequest(std::string key, size_t oc);

        char get_type() const override;

        std::vector<uint8_t> get_result();
    };

    CtCConnection(int fd, sockaddr_in other, Client& c, bool r);

    void add_request(std::shared_ptr<Request> request);

    void run() override;

    size_t hash() const override;

protected:
    Client& _client;
    bool _receiver;
    std::mutex _request_queue_mutex;
    std::queue<std::shared_ptr<Request>> _request_queue;
    std::mutex _waiting_requests_mutex;
    std::list<std::shared_ptr<Request>> _waiting_requests;

    void _as_client();

    void _as_receiver();

    void _send_keys();

    void _value_request_packet(Packet& packet, std::shared_ptr<ValueRequest> vrequest) const;

    void _push_request_packet(Packet& packet, std::shared_ptr<PushRequest> prequest) const;

    void _operation_request_packet(Packet& packet, std::shared_ptr<OperationRequest> orequest) const;

    ParseResult _update_keys(Packet &packet);

    ParseResult _distributed_insert(Packet &packet);

    ParseResult _parse_data(Packet& packet) override;

    int _respond(Packet& msg) override;

    void _check_requests();

    ParseResult _parse_value_response(Packet &p);

    ParseResult _parse_op_response(Packet &p);

    Packet _get_requested_value(std::string key);

    Packet _perform_operation(std::string key, size_t opcode);
};
