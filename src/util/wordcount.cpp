#include <iostream>
#include <iomanip>
#include <fstream>

#include "util/wordcount.h"
#include "network/network.h"

WordCount::CounterRower::CounterRower() : _word_map() {}

bool WordCount::CounterRower::accept(Row &r) {
    for(size_t c = 0; c < r.width(); ++c) {
        if (r.col_type(c) == 'S') {
            auto maybe_str = r.get_string(c);
            if(maybe_str) {
                auto it = _word_map.find(*maybe_str);
                if(it != _word_map.end()) {
                    ++_word_map[it->first];
                } else {
                    _word_map.insert({*maybe_str, 1});
                }
            }
        }
    }
    return true;
}

void WordCount::CounterRower::join(std::shared_ptr<Rower> other) {
    auto wcr = std::dynamic_pointer_cast<CounterRower>(other);
    if(wcr) {
        // this is also O(n)
        for(auto o_iter = wcr->_word_map.begin(); o_iter != wcr->_word_map.end(); ++o_iter){
            auto local_iter = this->_word_map.find(o_iter->first);
            if(local_iter != this->_word_map.end()) {
                this->_word_map[local_iter->first] += o_iter->second;
            } else {
                this->_word_map.insert({o_iter->first, o_iter->second});
            }
        }
    }
}


void WordCount::CounterRower::print() const {
    for(auto iter = _word_map.begin(); iter != _word_map.end(); ++iter){
        std::cout <<iter->first <<": " <<iter->second <<std::endl;
    }
}

std::shared_ptr<Object> WordCount::CounterRower::clone() const {
    return std::make_shared<WordCount::CounterRower>();
}

size_t WordCount::CounterRower::hash() const {
    size_t hash = _word_map.size();
    for(auto iter = _word_map.begin(); iter != _word_map.end(); ++iter){
        hash += iter->second ^ iter->first.size();
    }
    return hash;
}

bool WordCount::CounterRower::equals(const Object* other) const {
    auto cr = dynamic_cast<const WordCount::CounterRower *>(other);
    if(cr){
        return this->_word_map == cr->_word_map;
    }
    return false;
}


WordCount::WordCount() : Application(), _ip(nullptr), _server_ip(nullptr),
_filename(nullptr), _server_port(SERVER_PORT), _mode(Mode::NONE),
_key(std::string("wc_df")) {}

void WordCount::_read_in_file() const {
    std::unique_ptr<StringColumn> str_col = std::make_unique<StringColumn>();

    auto stream = std::fstream(_filename, std::fstream::in);
    std::string line;
    std::string word;
    while(!stream.eof() && !stream.fail() && !stream.bad()) {
        std::getline(stream, line);
        for(size_t i = 0; i < line.size(); ++i){
            if(!isspace(line[i])) {
                 word += line[i];
            } else if(!word.empty()) {
                str_col->push_back(word);
                word.clear();
            }
        }
        line.clear();
    }
    std::shared_ptr<DataFrame> df = std::make_shared<DataFrame>();
    df->add_column(std::move(str_col));
    KVStore::get_instance().set(_key, df);
}

void WordCount::_server() {
    const char *ip = DEFAULT_IP;
    if(_ip) {
        ip = _ip;
    }
    if(_server_ip) {
        ip = _server_ip;
    }
    assert(Server::init(ip, _server_port));
    Server::get_instance().lock()->listen_on_socket(10);
}

void WordCount::_counter() {
    const char *ip = DEFAULT_IP;
    const char *sip = DEFAULT_IP;
    if(_ip) ip = _ip;
    if(_server_ip) sip = _server_ip;

    assert(Client::init(ip, sip, _server_port));
    std::thread cthread([]{ Client::get_instance().lock()->listen_on_socket(10); });
    auto df = KVStore::get_instance().get_or_wait(_key);
    assert(df);
    WordCount::CounterRower cr;
    df->pmap(cr);
    cr.print();
    cthread.join();
}

void WordCount::_reader() {
    const char *ip = DEFAULT_IP;
    const char *sip = DEFAULT_IP;
    if(_ip) ip = _ip;
    if(_server_ip) sip = _server_ip;

    assert(Client::init(ip, sip, _server_port));
    std::thread cthread([]{ Client::get_instance().lock()->listen_on_socket(10); });
    if(!_filename) {
        std::cout <<"Filename required!" <<std::endl;
        Client::get_instance().lock()->request_teardown();
        cthread.join();
        exit(1);
    }
    try {
        this->_read_in_file();
    } catch(const std::exception& e) {
        std::cout <<"Error reading file: " <<e.what() <<std::endl;
        Client::get_instance().lock()->request_teardown();
        cthread.join();
        exit(1);
    }
    cthread.join();
}

void WordCount::_help(const char* bin_name) const {
    std::cout <<std::left <<std::setw(20) <<"Help:" <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"Usage: " <<bin_name <<" <options>" <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"Options:" <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--server:" 
        <<std::setw(20) <<"Run this node as the server." <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--reader:" 
        <<std::setw(20) <<"Run this node as the reader" <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--counter:" 
        <<std::setw(20) <<"Run this node as the counter." <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--address, -ip:" 
        <<std::setw(20) <<"Set the address of this node (Default: " <<DEFAULT_IP <<")." <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--server_address, -sip:" 
        <<std::setw(20) <<"Set the address of the server, will override --address if this node is the server. (Default: " <<DEFAULT_IP <<")." <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--port, -p:" 
        <<std::setw(20) <<"Set the port of the server (Default: " <<DEFAULT_SERVER_PORT <<")." <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--file, -f:" 
        <<std::setw(20) <<"Set the file to be read in." <<std::endl;
    exit(0);
}

void WordCount::parse_arguments(int argc, char **argv) {
    for(int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "--help") == 0){
            this->_help(argv[0]);
        }else if(strcmp(argv[i], "--server") == 0){
            _mode = SERVER;
        } else if(strcmp(argv[i], "--reader") == 0){
            _mode = READER;
        } else if(strcmp(argv[i], "--counter") == 0){
            _mode = COUNTER;
        } else if(strcmp(argv[i], "--address") == 0
                || strcmp(argv[i], "-ip") == 0){
            _ip = argv[++i];
        } else if(strcmp(argv[i], "--server_address") == 0
                ||strcmp(argv[i], "-sip") == 0){
            _server_ip = argv[++i];
        } else if(strcmp(argv[i], "--port") == 0
                || strcmp(argv[i], "-p") == 0) {
            _server_port = std::stoi(argv[++i]);
        } else if(strcmp(argv[i], "--file") == 0
                || strcmp(argv[i], "-f") == 0){
            _filename = argv[++i];
        } else {
            std::cout <<"Unrecognized Option: " <<argv[i] <<std::endl;
        }
    }
}

void WordCount::_run() {
    switch(_mode) {
        case Mode::NONE:
            std::cout <<"No Mode Specified!";
            exit(1);
            break;
        case Mode::SERVER:
            _server();
            break;
        case Mode::READER:
            _reader();
            break;
        case Mode::COUNTER:
            _counter();
            break;
        default:
            assert(false);
    }
}
