#include <cstring>
#include <assert.h>

#include "data/dataframe.h"
#include "network/ctc_connection.h"
#include "network/socket_util.h"

CtCConnection::ValueRequest::ValueRequest(std::string k) : key(k), mutex(), cv(), value(nullptr) {}

CtCConnection::CtCConnection(int fd, sockaddr_in other, Client& c, bool r) : Connection(fd, other),
_client(c), _receiver(r) {
    p("CTC Created!").p('\n');
}

CtCConnection::~CtCConnection() {
    std::unique_lock<std::mutex> rql(_request_queue_mutex);
    while(!_request_queue.empty()) {
        auto request = _request_queue.front();
        _request_queue.pop(); // remove above request from queue
        std::unique_lock<std::mutex> rl(request->mutex);
        request->value = nullptr;
        request->cv.notify_all();
    }
    rql.unlock();

    std::unique_lock<std::mutex> wrl(_waiting_requests_mutex);
    while(!_waiting_requests.empty()) {
        auto request = _waiting_requests.front();
        _waiting_requests.pop_front();
        std::unique_lock<std::mutex> rl(request->mutex);
        request->value = nullptr;
        request->cv.notify_all();
    }
    wrl.unlock();
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
        p("_check_requests ").p("Request Found: ").pln(request->key);

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
    if(_receiver){
        this->_as_receiver();
    } else {
        this->_as_client();
    }

    while(!this->is_finished() && this->dog_is_alive()){
        if(this->receive_and_parse()) {
            this->feed_dog();
            this->_client.feed_dog();
        }
        this->_send_keys();
        this->_check_requests();
        /* std::this_thread::sleep_for(std::chrono::milliseconds(100)); */
        std::this_thread::yield();
    }
    this->receive_and_parse();

    this->_finished = true;
}

void CtCConnection::_as_client() {
    pln("As Client");
    this->connect_to_target(this->_conn_other);
    pln("Connected to target!");
    Packet p;
    p.type = Packet::Type::ID;
    if(!this->_send_packet(p)){
        pln("Failed to send!");
        this->_finished = true;
    }
}

void CtCConnection::_as_receiver(){
    p("As Receiver").p('\n');
}

void CtCConnection::_send_keys(){
    auto key_set = KVStore::get_instance().get_local_keys();

    Packet key_packet;
    key_packet.type = Packet::Type::KEY_LIST;
    for(std::string key : key_set) {
        std::vector<uint8_t> serialized = Serializable::serialize<std::string>(key);
        key_packet.value.insert(key_packet.value.end(),
                                serialized.begin(),
                                serialized.end());
    }

    if(!this->_send_packet(key_packet)){
        pln("Failed to send!");
        this->_finished = true;
    }
}

ParseResult CtCConnection::_parse_data(Packet& packet) {
    if(packet.type == Packet::Type::VALUE_RESPONSE) {
        return this->_parse_request_response(packet);
    } else if(packet.type == Packet::Type::VALUE_REQUEST) {
        return ParseResult::Response;
    } else if(packet.type == Packet::Type::KEY_LIST){
        return this->_update_keys(packet);
    }

    return Connection::_parse_data(packet);
}

ParseResult CtCConnection::_parse_request_response(Packet &packet) {
    if(packet.type != Packet::Type::VALUE_RESPONSE) return ParseResult::ParseError;
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
}

ParseResult CtCConnection::_update_keys(Packet& packet){
    /* pln("_update_keys"); */
    if(packet.type != Packet::Type::KEY_LIST) return ParseResult::ParseError;
    size_t pos = 0;

    while(pos < packet.value.size()){
        KVStore::Key key(Serializable::deserialize<std::string>(packet.value, pos),
                         this->get_conn_other());
        KVStore::get_instance().add_nonlocal(key);
    }
    return ParseResult::Success;
}

int CtCConnection::_respond(Packet& msg) {
    if(msg.type == Packet::Type::ASK_FOR_ID){
        Packet packet = _client.get_registration_packet();
        packet.type = Packet::Type::ID;
        if(!this->_send_packet(packet)){
            p("Failed to respond!").p('\n');
        }
        return 0;
    } else if (msg.type == Packet::Type::VALUE_REQUEST){
        size_t pos = 0;
        std::string key = Serializable::deserialize<std::string>(msg.value, pos);

        Packet response = this->_get_requested_value(key);
        if(this->_send_packet(response)){
            return 0;
        }
    }
    return -1;
}

Packet CtCConnection::_get_requested_value(std::string key) {
    p("_get_requested_value ").p("Value Requsted: ").pln(key);
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
    return response;
}

size_t CtCConnection::hash() const {
    return reinterpret_cast<uintptr_t>(this);
}
