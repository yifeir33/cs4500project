#include <cstring>
#include <assert.h>

#include "data/dataframe.h"
#include "network/ctc_connection.h"

CtCConnection::ValueRequest::ValueRequest(std::string k) : key(k), mutex(), cv(), value(nullptr) {}

CtCConnection::CtCConnection(int fd, sockaddr_in other, Client& c, bool r) : Connection(fd, other),
_client(c), _receiver(r) {
    p("CTC Created!").p('\n');
}

void CtCConnection::add_request(std::shared_ptr<CtCConnection::ValueRequest> request) {
    std::lock_guard<std::mutex> lock(_request_queue_mutex);
    _request_queue.push(request);
}

void CtCConnection::_check_requests() {
    std::unique_lock<std::mutex> rql(_request_queue_mutex);
    while(!_request_queue.empty()){
        auto request = _request_queue.front();
        _request_queue.pop(); // remove above request from queue

        Packet packet;
        packet.type = Packet::Type::VALUE_REQUEST;

        std::unique_lock<std::mutex> rl(request->mutex);
        auto serialized_str = Serializable::serialize<std::string>(request->key);
        rl.unlock();
        packet.value.insert(packet.value.end(), serialized_str.begin(), serialized_str.end());

        std::unique_lock<std::mutex> wrl(_waiting_requests_mutex);
        this->_waiting_requests.push_back(request);
        wrl.unlock();

        rql.unlock();
        this->_send_packet(packet);
        rql.lock();
    }
}

void CtCConnection::run() {
    while(!this->is_finished() && this->dog_is_alive()){
        if(this->receive_and_parse()) {
            this->feed_dog();
            this->_client.feed_dog();
        }
        this->_check_requests();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    this->receive_and_parse();

    this->_finished = true;
}

void CtCConnection::_as_client() {
    p("As Client").p('\n');
    this->connect_to_target(this->_conn_other);
    p("Connected to target!").p('\n');
    // TODO
    while(!this->is_finished() && this->dog_is_alive()){
        if(this->receive_and_parse()) this->feed_dog();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void CtCConnection::_as_receiver(){
    p("As Receiver").p('\n');
    while(!this->is_finished() && this->dog_is_alive()){
        if(this->receive_and_parse()) {
            this->feed_dog();
            this->_client.feed_dog();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    this->receive_and_parse();
}

ParseResult CtCConnection::_parse_data(Packet& packet) {
    if(packet.type == Packet::Type::VALUE_RESPONSE) {
        // format of response: Key, Value
        bool success = packet.value[0];
        size_t pos = 1;
        std::string k = Serializable::deserialize<std::string>(packet.value, pos);
        std::shared_ptr<DataFrame> df = nullptr;
        if(success) {
            df = std::make_shared<DataFrame>(Serializable::deserialize<DataFrame>(packet.value, pos));
        }

        std::lock_guard<std::mutex> request_lock(_waiting_requests_mutex);
        for(auto it = _waiting_requests.begin(); it != _waiting_requests.end(); ++it){
            std::lock_guard<std::mutex> rlock((*it)->mutex);
            if(k == (*it)->key) {
                (*it)->value = df;
                _waiting_requests.erase(it);
                (*it)->cv.notify_all();
                return ParseResult::Success;
            }
        }
        return ParseResult::ParseError;
    } else if(packet.type == Packet::Type::VALUE_REQUEST) {
        return ParseResult::Response;
    }
    return Connection::_parse_data(packet);
}

int CtCConnection::_respond(Packet& msg) {
    if(msg.type == Packet::Type::ASK_FOR_ID){
        auto packet = _client.get_registration_packet();
        packet->type = Packet::Type::ID;
        if(!this->_send_packet(*packet)){
            p("Failed to respond!").p('\n');
        }
        return 0;
    } else if (msg.type == Packet::Type::VALUE_REQUEST){
        size_t pos = 0;
        std::string key = Serializable::deserialize<std::string>(msg.value, pos);

        auto value = KVStore::get_instance().get_local(KVStore::Key(key));

        Packet response;
        response.type = Packet::Type::VALUE_RESPONSE;
        response.value.push_back(!!value);
        auto serialized_key = Serializable::serialize<std::string>(key);
        response.value.insert(response.value.end(), serialized_key.begin(), serialized_key.end());
        if(value) {
            auto serialized_value = value->serialize();
            response.value.insert(response.value.end(), serialized_value.begin(), serialized_value.end());
        }
        this->_send_packet(response);
    }
    return -1;
}

size_t CtCConnection::hash() const {
    return reinterpret_cast<uintptr_t>(this);
}
