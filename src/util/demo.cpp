#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <cassert>

#include "util/demo.h"
#include "network/network.h"
#include "data/dataframe.h"

Demo::SumRower::SumRower() : _sum(0) {}

bool Demo::SumRower::accept(Row& r) {
    for(size_t c = 0; c < r.width(); ++c) {
        if(r.col_type(c) == 'F'){
            auto d = r.get_double(c);
            if(d) _sum += *d;
        }
    }
    return true;
}

void Demo::SumRower::join(std::shared_ptr<Rower> other) {
    auto osr = std::dynamic_pointer_cast<Demo::SumRower>(other);
    if(osr) {
        _sum += osr->_sum;
    }
}

std::shared_ptr<Object> Demo::SumRower::clone() const {
    return std::make_shared<SumRower>();
}

bool Demo::SumRower::equals(const Object *other) const {
    return this == other;
}

size_t Demo::SumRower::hash() const {
    return reinterpret_cast<size_t>(this);
}

Demo::Demo() : Application(), _ip(nullptr), _server_ip(nullptr),
_server_port(DEFAULT_SERVER_PORT), _mode(NONE) {}

void Demo::_producer() {
    std::cout << "Producer" <<std::endl;
    constexpr size_t SZ = 100 * 1000;
    double vals[SZ] = {};

    double sum = 0;
    for(size_t i = 0; i < SZ; ++i) sum += (vals[i] = i);
    std::cout <<"Sum: " <<sum <<std::endl;

    auto df = DataFrame::from_array(KVStore::Key("main"), vals, SZ);
    std::cout <<"(100, 0): " <<*df->get_double(0, 100) <<std::endl;
    DataFrame::from_scalar(KVStore::Key("check"), sum);
}

void Demo::_counter() {
    std::cout << "Counter" <<std::endl;
    std::shared_ptr<DataFrame> df = KVStore::get_instance().get_or_wait(KVStore::Key("main"),
                                                                        1000 * 60); // 1 minute timeout
    if(!df) std::cout <<"get_or_wait timeout" <<std::endl;
    std::cout <<"(100, 0): " <<*df->get_double(0, 100) <<std::endl;
    SumRower sr;
    df->pmap(sr);
    DataFrame::from_scalar(KVStore::Key("verify"), sr._sum);
    std::cout <<"Counter Sum: " <<sr._sum <<std::endl;
}

void Demo::_summarizer() {
    std::cout << "Summarizer" <<std::endl;
    std::shared_ptr<DataFrame> result = KVStore::get_instance().get_or_wait(KVStore::Key("verify"),
                                                                            1000 * 60); // 1 minute timeout
    if(!result) std::cout <<"get_or_wait timeout" <<std::endl;
    std::shared_ptr<DataFrame> expected = KVStore::get_instance().get_or_wait(KVStore::Key("check"),
                                                                              1000 * 60); // 1 minute timeout
    if(!expected) std::cout <<"get_or_wait timeout" <<std::endl;

    std::cout<<"Demo Result:" <<std::endl;
    std::cout<<"Expected: " <<*expected->get_double(0, 0) <<std::endl;
    std::cout<<"Actual: " <<*result->get_double(0, 0) <<std::endl;
    std::cout <<(*expected->get_double(0, 0) - *result->get_double(0, 0) < 0.001 ? "Success" : "Failure") <<std::endl;
    Client::get_instance().lock()->request_teardown();
}

void Demo::_run() {
    if(_mode == NONE) {
        std::cout << "No mode provided!" <<std::endl;
        exit(1);
    }
    if(_mode == SERVER){
        const char *ip = DEFAULT_IP;
        if(_ip) {
            ip = _ip;
        }
        if(_server_ip) {
            ip = _server_ip;
        }
        assert(Server::init(ip, _server_port));
        // this blocks the thread indefinetly,
        // but no work is done serverside
        Server::get_instance().lock()->listen_on_socket(30);
        return;
    } else {
        const char *ip = DEFAULT_IP;
        const char *sip = DEFAULT_IP;
        if(_ip) ip = _ip;
        if(_server_ip) sip = _server_ip;
        assert(Client::init(ip, sip, _server_port));
    }

    // this blocks so we put it in another thread
    std::thread cthread([]{ Client::get_instance().lock()->listen_on_socket(10); });
    std::cout <<"Client Started!" <<std::endl;

    switch(_mode){
        case PRODUCER:
            this->_producer();
            break;
        case SUMMARIZER:
            this->_summarizer();
            break;
        case COUNTER:
            this->_counter();
            break;
        default:
            assert(false); // unreachable
            break;
    }

    // after work is completed we wait for client to end
    cthread.join();
}

void Demo::_help(const char* bin_name) const {
    std::cout <<std::left <<std::setw(20) <<"Help:" <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"Usage: " <<bin_name <<" <options>" <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"Options:" <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--server:" 
        <<std::setw(20) <<"Run this node as the server." <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--producer:" 
        <<std::setw(20) <<"Run this node as the producer" <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--counter:" 
        <<std::setw(20) <<"Run this node as the counter." <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--summarizer" 
        <<std::setw(20) <<"Run this node as the summarizer." <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--address, -ip:" 
        <<std::setw(20) <<"Set the address of this node (Default: " <<DEFAULT_IP <<")." <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--server, -sip:" 
        <<std::setw(20) <<"Set the address of the server, will override --address if this node is the server. (Default: " <<DEFAULT_IP <<")." <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--port, -p:" 
        <<std::setw(20) <<"Set the port of the server (Default: " <<DEFAULT_SERVER_PORT <<")." <<std::endl;
    exit(0);
}

void Demo::parse_arguments(int argc, char **argv) {
    for(int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "--help") == 0){
            this->_help(argv[0]);
        }else if(strcmp(argv[i], "--server") == 0){
            _mode = SERVER;
        } else if(strcmp(argv[i], "--producer") == 0){
            _mode = PRODUCER;
        } else if(strcmp(argv[i], "--counter") == 0){
            _mode = COUNTER;
        } else if(strcmp(argv[i], "--summarizer") == 0){
            _mode = SUMMARIZER;
        } else if(strcmp(argv[i], "--address") == 0
                || strcmp(argv[i], "-ip")){
            _ip = argv[++i];
        } else if(strcmp(argv[i], "--server")
                ||strcmp(argv[i], "-sip")){
            _server_ip = argv[++i];
        } else if(strcmp(argv[i], "--port")
                || strcmp(argv[i], "-p")) {
            _server_port = std::stoi(argv[++i]);
        } else {
            std::cout <<"Unrecognized Option: " <<argv[i] <<std::endl;
        }
    }
}
