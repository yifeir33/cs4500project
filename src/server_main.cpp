
#include "network/server.h"

int main(int argc, char** argv){
    // TODO: write arg parsing
    Server::init("127.0.0.1", 8001);
    Server::get_instance().lock()->listen_on_socket(30);
}
