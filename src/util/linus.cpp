#include <iostream> 
#include <iomanip>

#include "network/network.h"
#include "util/linus.h"
#include "util/linus_rowers.h"
#include "adapter/sorer_dataframe_adapter.h"

Linus::Linus() : Application(), _ip(nullptr), _server_ip(nullptr), _filename(),
_server_port(SERVER_PORT), _mode(Mode::NONE) {}

std::shared_ptr<DataFrame> Linus::_read_in_file(KVStore::Key k, std::string fn) const {
    std::cout <<"Reading in file: " <<fn <<std::endl;
    auto df = SorerDataframeAdapter::parse_file(fn);
    std::cout <<"File Read in!" <<std::endl;
    assert(df);
    std::cout <<"Valid DF created!" <<std::endl;
    KVStore::get_instance().set(k, df);
    return df;
}

void Linus::_projects() {
    std::cout <<"Projects" <<std::endl;
    // read in projects file
    auto pdf = this->_read_in_file(KVStore::Key("projects"), _filename);
    std::thread network_thread([]{ Client::get_instance().lock()->listen_on_socket(30); });
    // this file is useless lmao
    std::cout <<"Projects Finished!" <<std::endl;
    network_thread.join();
}

void Linus::_users() {
    std::cout <<"Users" <<std::endl;
    // read in users file
    auto udf = this->_read_in_file(KVStore::Key("users"), _filename);
    std::cout <<"Users Read in" <<std::endl;

    std::thread network_thread([]{ Client::get_instance().lock()->listen_on_socket(30); });

    // find linus and add him to a local dataframe
    for(size_t r = 0; r < udf->nrows(); ++r) {
        // look for linus
        auto name = udf->get_string(1, r);
        if(name && *name == std::string("torvalds")) {
            std::cout <<"Linus UUID: " <<*udf->get_int(0, r) <<std::endl;
            DataFrame::from_scalar(KVStore::Key("linus_uuid"), *udf->get_int(0, r));
            break; // found linus
        }
    }

    // get the set of projects linus worked on and start creating degrees of users
    // convert from uuids to names
    std::string uuk = std::string("uuids_degree_");
    std::string uk = std::string("degree");
    for(size_t degree = 1; degree <= 7; ++degree) {
        auto uudf = KVStore::get_instance().get_or_wait(KVStore::Key(uuk + std::to_string(degree)));
        assert(uudf);
        std::cout <<"Got UUIDs for degree " <<degree <<std::endl;

        // generate list of user names
        UUIDsToNamesFilter uunf(uudf);
        udf->pmap(uunf);
        auto degree_names = uunf.finish_filter();
        KVStore::get_instance().set(KVStore::Key(uk + std::to_string(degree)),
                                    degree_names);

        std::cout <<'\n' <<"Degree " <<degree <<":" <<'\n' <<std::endl;
        degree_names->print();
    }
    std::cout <<"Users Finished!" <<std::endl;
    network_thread.join();
}

void Linus::_commits() {
    std::cout <<"Commits" <<std::endl;
    // read in commits file
    auto cdf = this->_read_in_file(KVStore::Key("commits"), _filename);
    std::cout <<"Commits read in!" <<std::endl;
    std::thread network_thread([]{ Client::get_instance().lock()->listen_on_socket(30); });
    // get linus uuid
    auto luuid_df = KVStore::get_instance().get_or_wait(KVStore::Key("linus_uuid"));
    assert(luuid_df);
    std::cout <<"Got Linus UUID" <<std::endl;

    // generate set of projects linus worked on
    UUIDsToProjectsFilter lpf(luuid_df);
    cdf->pmap(lpf);
    auto project_frame = lpf.finish_filter();
    KVStore::get_instance().set(KVStore::Key("linus_projects"), project_frame);
    std::cout <<"Linus Projects Generated!" <<std::endl;

    // now we generate the user degrees
    std::string uuk = std::string("uuids_degree_");
    std::string pk = std::string("projects_degree_");
    for(size_t degree = 1; degree <= 7; ++degree) {
        // store the list of uuids for each degree
        ProjectsToUUIDsFilter ptuuf(project_frame);
        cdf->pmap(ptuuf);
        auto degree_uuids = ptuuf.finish_filter();
        KVStore::get_instance().set(KVStore::Key(uuk + std::to_string(degree)),
                                    degree_uuids);
        std::cout <<"UUIDs Degree " <<degree <<" Generated!" <<std::endl;

        if(degree == 7) break;

        // now we regenerate the larger list of projects for the next degree
        UUIDsToProjectsFilter uutpf(degree_uuids);
        cdf->pmap(uutpf);
        project_frame = uutpf.finish_filter();
        KVStore::get_instance().set(KVStore::Key(pk + std::to_string(degree)),
                                    project_frame);
        std::cout <<"Projects Degree " <<degree + 1 <<" Generated!" <<std::endl;
    }

    std::cout <<"Commits Finished!" <<std::endl;
    network_thread.join();
}

void Linus::parse_arguments(int argc, char **argv) {
    for(int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "--help") == 0){
            this->_help(argv[0]);
        }else if(strcmp(argv[i], "--server") == 0){
            _mode = SERVER;
        } else if(strcmp(argv[i], "--commits") == 0){
            _mode = COMMITS;
            if(_filename.empty()) _filename = std::string("commits.ltgt");
        } else if(strcmp(argv[i], "--users") == 0){
            _mode = USERS;
            if(_filename.empty()) _filename = std::string("users.ltgt");
        } else if(strcmp(argv[i], "--projects") == 0){
            _mode = PROJECTS;
            if(_filename.empty()) _filename = std::string("projects.ltgt");
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
            _filename = std::string(argv[++i]);
        } else {
            std::cout <<"Unrecognized Option: " <<argv[i] <<std::endl;
        }
    }
}
void Linus::_help(const char* bin_name) const {
    std::cout <<std::left <<std::setw(20) <<"Help:" <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"Usage: " <<bin_name <<" <options>" <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"Options:" <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--server:" 
        <<std::setw(20) <<"Run this node as the server." <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--commits:" 
        <<std::setw(20) <<"Run this node as the one reading in and working with the commits file" <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--users:" 
        <<std::setw(20) <<"Run this node as the one reading in and working with the users file" <<std::endl;
    std::cout <<std::left <<std::setw(20) <<"--projects:" 
        <<std::setw(20) <<"Run this node as the one reading in and working with the projects file" <<std::endl;
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

void Linus::_run() {
    if(_mode == Mode::NONE) {
        std::cout << "Failed to select a mode!" <<std::endl;
        this->_help(""); // exits
    } else if(_mode == Mode::SERVER) {
        const char *ip = DEFAULT_IP;
        if(_ip) {
            ip = _ip;
        }
        if(_server_ip) {
            ip = _server_ip;
        }

        Server::init(ip, _server_port);
        Server::get_instance().lock()->listen_on_socket(50);
    } else {
        const char *ip = DEFAULT_IP;
        const char *sip = DEFAULT_IP;
        if(_ip) ip = _ip;
        if(_server_ip) sip = _server_ip;

        Client::init(ip, sip, _server_port);

        switch(_mode) {
            case PROJECTS:
                _projects();
                break;
            case USERS:
                _users();
                break;
            case COMMITS:
                _commits();
                break;
            default:
                assert(false);
        }

    }
}
