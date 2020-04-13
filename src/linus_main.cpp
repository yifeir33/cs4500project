#include <signal.h>

#include "util/linus.h"

int main(int argc, char** argv) {
    // ignore sigpipe
    signal(SIGPIPE, SIG_IGN);

    Linus linus;
    linus.parse_arguments(argc, argv);
    linus.start();
    return 0;
}
