
#include "network/client.h"

int main(int argc, char** argv){
    // TODO: write arg parsing
    Client::init("127.0.0.1", "127.0.0.1", 8001);
    Client::get_instance().lock()->listen_on_socket(20);
}
