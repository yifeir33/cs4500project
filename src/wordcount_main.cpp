#include <signal.h>

#include "util/wordcount.h"

int main(int argc, char** argv) {
    // ignore sigpipe
    signal(SIGPIPE, SIG_IGN);

    WordCount wc;
    wc.parse_arguments(argc, argv);
    wc.start();
    return 0;
}
