#include <signal.h>

#include "util/demo.h"

int main(int argc, char** argv) {
    // ignore sigpipe
    signal(SIGPIPE, SIG_IGN);

    Demo d;
    d.parse_arguments(argc, argv);
    d.start();
    return 0;
}
