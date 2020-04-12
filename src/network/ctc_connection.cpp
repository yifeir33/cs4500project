#include <cstring>
#include <assert.h>

#include "data/dataframe.h"
#include "network/ctc_connection.h"
#include "network/socket_util.h"

CtCConnection::ValueRequest::ValueRequest(std::string k) : key(k), mutex(), cv(), value(nullptr) {}

char CtCConnection::ValueRequest::get_type() const { return 'V'; }

CtCConnection::PushRequest::PushRequest(std::string k, std::vector<DistributedStore::MetaData> m,
                                        std::vector<std::string> lks,
                                        std::vector<std::shared_ptr<DataFrame>> dfs)
: dkey(k), md(m), lkeys(lks), ldfs(dfs){}

char CtCConnection::PushRequest::get_type() const {
    return 'P';
}

CtCConnection::OperationRequest::OperationRequest(std::string key, size_t oc) 
: key(key), opcode(oc), mutex(), cv(), result() {}

char CtCConnection::OperationRequest::get_type() const {
    return 'O';
}

CtCConnection::CtCConnection(int fd, sockaddr_in other, Client& c, bool r) : Connection(fd, other),
_client(c), _receiver(r) {
    p("CTC Created!").p('\n');
}

void CtCConnection::add_request(std::shared_ptr<CtCConnection::Request> request) {
    std::lock_guard<std::mutex> lock(_request_queue_mutex);
    _request_queue.push(request);
}

void CtCConnection::_value_request_packet(Packet& packet, std::shared_ptr<ValueRequest> vrequest) const {
    // value request
    packet.type = Packet::Type::VALUE_REQUEST;

    std::unique_lock<std::mutex> rl(vrequest->mutex);
    auto serialized_str = Serializable::serialize<std::string>(vrequest->key);
    rl.unlock();
    packet.value.insert(packet.value.end(), serialized_str.begin(), serialized_str.end());
}

void CtCConnection::_push_request_packet(Packet& packet, std::shared_ptr<PushRequest> prequest) const {
    packet.type = Packet::Type::PUSH_REQUEST;

    // distributed key
    std::vector<uint8_t> serialized = Serializable::serialize<std::string>(prequest->dkey);
    packet.value.insert(packet.value.end(), serialized.begin(), serialized.end());

    // metadata
    serialized = Serializable::serialize<size_t>(prequest->md.size());
    packet.value.insert(packet.value.end(), serialized.begin(), serialized.end());
    for(auto m : prequest->md) {
        serialized = m.serialize();
        packet.value.insert(packet.value.end(), serialized.begin(), serialized.end());
    }

    // local keys
    serialized = Serializable::serialize<size_t>(prequest->lkeys.size());
    packet.value.insert(packet.value.end(), serialized.begin(), serialized.end());
    for(auto lk : prequest->lkeys) {
        serialized = Serializable::serialize<std::string>(lk);
        packet.value.insert(packet.value.end(), serialized.begin(), serialized.end());
    }

    // dataframes
    serialized = Serializable::serialize<size_t>(prequest->ldfs.size());
    packet.value.insert(packet.value.end(), serialized.begin(), serialized.end());
    for(auto df : prequest->ldfs) {
        serialized = df->serialize();
        packet.value.insert(packet.value.end(), serialized.begin(), serialized.end());
    }
}

void CtCConnection::_operation_request_packet(Packet& packet, std::shared_ptr<OperationRequest> orequest) const {
    packet.type = Packet::Type::OP_REQUEST;
    std::lock_guard<std::mutex> guard(orequest->mutex);

    std::vector<uint8_t> serialized = Serializable::serialize<std::string>(orequest->key);
    packet.value.insert(packet.value.end(), serialized.begin(), serialized.end());

    serialized = Serializable::serialize<size_t>(orequest->opcode);
    packet.value.insert(packet.value.end(), serialized.begin(), serialized.end());
}

void CtCConnection::_check_requests() {
    std::unique_lock<std::mutex> rql(_request_queue_mutex);
    while(!_request_queue.empty()){
        auto request = _request_queue.front();
        _request_queue.pop(); // remove above request from queue
        Packet packet;

        if(request->get_type() == 'V') {
            auto vrequest = std::static_pointer_cast<ValueRequest>(request);
            this->_value_request_packet(packet, vrequest);

            std::unique_lock<std::mutex> wrl(_waiting_requests_mutex);
            this->_waiting_requests.push_back(vrequest);
            wrl.unlock();
        } else if(request->get_type() == 'P') {
            // push request
            auto prequest = std::static_pointer_cast<PushRequest>(request);
            this->_push_request_packet(packet, prequest);
            // don't need to wait for response
        } else if(request->get_type() == 'O') {
            // operation request
            auto orequest = std::static_pointer_cast<OperationRequest>(request);
            this->_operation_request_packet(packet, orequest);

            std::unique_lock<std::mutex> wrl(_waiting_requests_mutex);
            this->_waiting_requests.push_back(orequest);
            wrl.unlock();
        }

        rql.unlock();
        this->_send_packet(packet);
        rql.lock();
    } // while
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
        this->_finished = true;
    }
}

ParseResult CtCConnection::_distributed_insert(Packet &packet) {
    // deserialize
    size_t pos = 0;
    // distributed key
    std::string dkey = Serializable::deserialize<std::string>(packet.value, pos);

    // metadata
    size_t md_cnt = Serializable::deserialize<size_t>(packet.value, pos);
    std::vector<DistributedStore::MetaData> md;
    for(size_t i = 0; i < md_cnt; ++i) {
        md.push_back(Serializable::deserialize<DistributedStore::MetaData>(packet.value, pos));
    }

    // local keys
    size_t local_cnt = Serializable::deserialize<size_t>(packet.value, pos);
    std::vector<std::string> lkeys;
    for(size_t i = 0; i < local_cnt; ++i) {
        lkeys.push_back(Serializable::deserialize<std::string>(packet.value, pos));
    }

    // dataframes
    size_t df_cnt = Serializable::deserialize<size_t>(packet.value, pos);
    std::vector<std::shared_ptr<DataFrame>> ldfs;
    for(size_t i = 0; i < df_cnt; ++i) {
        ldfs.push_back(std::make_shared<DataFrame>(Serializable::deserialize<DataFrame>(packet.value, pos)));
    }

    DistributedStore::get_instance().insert(dkey, md, lkeys, ldfs);
    return ParseResult::Success;
}

ParseResult CtCConnection::_parse_data(Packet& packet) {
    if(packet.type == Packet::Type::VALUE_RESPONSE) {
        return this->_parse_value_response(packet);
    } else if(packet.type == Packet::Type::VALUE_REQUEST) {
        return ParseResult::Response;
    } else if(packet.type == Packet::Type::KEY_LIST){
        return this->_update_keys(packet);
    } else if(packet.type == Packet::Type::PUSH_REQUEST) {
        return this->_distributed_insert(packet);
    } else if(packet.type == Packet::Type::OP_REQUEST) {
        return ParseResult::Response;
    } else if(packet.type == Packet::Type::OP_RESPONSE) {
        return this->_parse_op_response(packet);
    }

    return Connection::_parse_data(packet);
}

ParseResult CtCConnection::_parse_op_response(Packet &p) {
    size_t pos = 0;
    std::string key = Serializable::deserialize<std::string>(p.value, pos);
    size_t opcode = Serializable::deserialize<size_t>(p.value, pos);

    // find request in waiting requests
    std::lock_guard<std::mutex> request_lock(_waiting_requests_mutex);
    for(auto it = _waiting_requests.begin(); it != _waiting_requests.end(); ++it){
        if((*it)->get_type() == 'O') {
            auto orequest = std::static_pointer_cast<OperationRequest>(*it);
            std::lock_guard<std::mutex> rlock(orequest->mutex);
            if(key == orequest->key && opcode == orequest->opcode) {
                orequest->result.insert(orequest->result.end(),
                                        p.value.begin() + pos,
                                        p.value.end());
                _waiting_requests.erase(it);
                orequest->cv.notify_all();
                return ParseResult::Success;
            }
        }
    }
    return ParseResult::ParseError;
}

ParseResult CtCConnection::_parse_value_response(Packet &packet) {
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
        if((*it)->get_type() == 'V') {
            auto vrequest = std::static_pointer_cast<ValueRequest>(*it);
            std::lock_guard<std::mutex> rlock(vrequest->mutex);
            if(k == vrequest->key) {
                vrequest->value = df;
                _waiting_requests.erase(it);
                vrequest->cv.notify_all();
                return ParseResult::Success;
            }
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
    } else if(msg.type == Packet::Type::OP_REQUEST) {
        size_t pos = 0;
        std::string key = Serializable::deserialize<std::string>(msg.value, pos);
        size_t opcode = Serializable::deserialize<size_t>(msg.value, pos);

        Packet response = this->_perform_operation(key, opcode);
        if(this->_send_packet(response)){
            return 0;
        }
    }
    return -1;
}

Packet CtCConnection::_perform_operation(std::string key, size_t opcode) {
    auto result = DistributedStore::get_instance().local_map(key, opcode);
    Packet packet;
    packet.type = Packet::Type::OP_RESPONSE;

    std::vector<uint8_t> serialized = Serializable::serialize<std::string>(key);
    packet.value.insert(packet.value.end(), serialized.begin(), serialized.end());

    serialized = Serializable::serialize<size_t>(opcode);
    packet.value.insert(packet.value.end(), serialized.begin(), serialized.end());

    /* serialized = Serializable::serialize<size_t>(result.size()); */
    /* packet.value.insert(packet.value.end(), serialized.begin(), serialized.end()); */

    packet.value.insert(packet.value.end(), result.begin(), result.end());

    return packet;
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
