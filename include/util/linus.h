#pragma once

#include <string>

#include "util/application.h"
#include "data/kvstore.h"

#define DEFAULT_IP   "127.0.0.1"
#define DEFAULT_SERVER_PORT 8001

class Linus : public Application {
private:
    enum Mode {
        NONE,
        SERVER,
        PROJECTS,
        USERS,
        COMMITS
    };

    const char *_ip;
    const char *_server_ip;
    std::string _filename;
    in_port_t _server_port;
    Mode _mode;

    std::shared_ptr<DataFrame> _read_in_file(KVStore::Key k, std::string fn) const;

    void _projects();

    void _users();

    void _commits();

protected:
    void _run() override;

    void _help(const char* bin_name) const override;

public:
    Linus();

    void parse_arguments(int argc, char** argv) override;
};
