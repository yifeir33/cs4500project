#pragma once

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <list>

#include "network/client.h"
#include "network/connection.h"

/** Concrete implementation of a Client-to-Client
 * worker that handles communication between two client nodes. */
class CtCConnection : public Connection {
public:
    /** A inner class used to request values from another client's KVStore.
     * It is used to communicate across threads what value is being requested,
     * and to allow the connection to notify the waiting thread that the
     * request has been fufilled. */
    class ValueRequest {
    public:
        /** The key whose associated value is being requested. */
        std::string key;
        /** A mutex protecting the value pointer of this class. It
         * is also associated with the condition variable. */
        std::mutex mutex;
        /** A condition variable so that the thread waiting for its request to
         * be fufilled can be notified when the request is fufilled. */
        std::condition_variable cv;
        /** A pointer which is set to the value associated with the key when
         * the request is received. If the value cannot be found, it is set
         * to a nullptr before the waiting thread is woken up. */
        std::shared_ptr<DataFrame> value;

        /** Constructor to initialize the various internals. */
        ValueRequest(std::string k);
    };

    /** Constructor that initializes the worker with the socket to use,
     * the address it is connected to/to connect to, the Client it represents,
     * and a boolean representing whether it was created by accepting a connection
     * (true) or needs to create a connection (false) */
    CtCConnection(int fd, sockaddr_in other, Client& c, bool r);

    /** Destructor - Clears the request queues by returning nullptr for all unfufilled
     * requests. */
    ~CtCConnection();

    /** Adds a request to the request queue for this connection worker to 
     * fufill. */
    void add_request(std::shared_ptr<ValueRequest> request);

    /** Implementation of the main event loop for this worker. Initially, iff it is a "client",
     * it connects to the given other node, and identifies itself. It then receives data,
     * updates the other client with what keys are stored locally in its KVStore, and
     * fufills requests for remote values stored on the endpoint node. */
    void run() override;

    /** Returns the hashcode for this object. */
    size_t hash() const override;

protected:
    /** Reference to the client object which this connection represents locally. */
    Client& _client;
    /** True if this is the "receiver", and so has accepted a connection from another
     * client, false if this is the "client", and needs to connect and identify
     * itself to the other client. */
    bool _receiver;
    /** Mutex protecting the request queue. */
    std::mutex _request_queue_mutex;
    /** The queue of waiting requests which have not yet been communicated to the
     * remote client. */
    std::queue<std::shared_ptr<ValueRequest>> _request_queue;
    /** Mutex protecting the list of waiting requests. */
    std::mutex _waiting_requests_mutex;
    /** The list of requests that have communicated to the remote node, but have
     * not yet received a response. */
    std::list<std::shared_ptr<ValueRequest>> _waiting_requests;

    /** Helper function to perform the required actions if _receiver is false.
     * In this case, it connects to the given target and identifies which client
     * it represents. */
    void _as_client();

    /** Helper function to perform the required actions if _receiver is true.
     * In this case, it does nothing. */
    void _as_receiver();

    /** Send the set of keys whose associated values are stored locally on this
     * node. */
    void _send_keys();

    /** Updates the list of remote keys with the latest set that the other
     * node claims to have. */
    ParseResult _update_keys(Packet &packet);

    /** Overriden parse data method which provides the correct parsing for 
     * Client to Client only packets. */
    ParseResult _parse_data(Packet& packet) override;

    /** Overriden respond method which provides the correct parsing and response for 
     * Client to Client only packets. */
    int _respond(Packet& msg) override;

    /** Checks the queue of requests and asks the other client for the value associated
     * with the key for each request it finds, and then moves the request to the
     * waiting requests list. */
    void _check_requests();

    /** Helper function that parses the response to a value request, and
     * updates the correct waiting request with the received value, wakes the
     * thread and removes it from the list of waiting requests. */
    ParseResult _parse_request_response(Packet &p);

    /** Helper function that parses a request for a value from a remote node,
     * gets the value from the local KVStore, and sends the response to the
     * remote node. */
    Packet _get_requested_value(std::string key);
};
