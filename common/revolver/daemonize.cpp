#include "revolver/daemonize.h"
#include <string.h>
#include <stdlib.h>

void daemonize(int argc, const char* argv[]) {
    if (argc < 2) return;
    for (int i = 1; i < argc; ++i) {
        if(!strcasecmp(argv[i], "-d")) {
            daemon(0, 0);
            break;
        }
    }
}