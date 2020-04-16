#pragma once

#include <string>

#include "util/application.h"
#include "data/kvstore.h"

#define DEFAULT_IP   "127.0.0.1"
#define DEFAULT_SERVER_PORT 8001

/**************************************************************************
 * Linus ::
 * This is an application created by extend the application class.
 * This demo class is what required in Milestone 5
 */
class Linus : public Application {
private:
    /*
     * The application contains 5 modes include:
     * none, server, projects, users, and commits.
     */
    enum Mode {
        NONE,
        SERVER,
        PROJECTS,
        USERS,
        COMMITS
    };
    /**
     * ip of the application.
     */
    const char *_ip;
    /**
     * the server of the ip.
     */
    const char *_server_ip;
    /**
     * The name of the file as input
     */
    std::string _filename;
    /**
     * The port of the server
     */
    in_port_t _server_port;
    /**
     * The mode that this applicatoin is currently on
     */
    Mode _mode;

    /**
     * The dataframe is produced after we read in the file,
     * @param k The key it belongs to
     * @param fn
     * @return the dataframe after we read the file
     */
    std::shared_ptr<DataFrame> _read_in_file(KVStore::Key k, std::string fn) const;

    /**
     *
     */
    void _projects();

    /**
     *
     */
    void _users();

    /**
     *
     */
    void _commits();

protected:
    void _run() override;
    /**
    * Explain the usage of each with providing the flag name.
    * @param bin_name the flag that client wants to get explained.
    */
    void _help(const char* bin_name) const override;

public:
    /**
     * Create a empty linus application.
     */
    Linus();
    /**
    * This method will take inputs from the client, it will switch the mode based on
    * the command that is provided, for the current application's help method.
    * @param argc the postion of the argument
    * @param argv the character of the argument
    */
    void parse_arguments(int argc, char** argv) override;
};
