#include "revolver/base_typedef.h"
#include "revolver/daemonize.h"
#include <stdlib.h>
#if (defined(OS_DARWIN) || (defined(OS_LINUX)))
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>

void daemonize(int argc, const char* argv[]) {
#if (defined(OS_DARWIN) || (defined(OS_LINUX)))
    if (argc < 2) return;
    for (int i = 1; i < argc; ++i) {
        if(!strcasecmp(argv[i], "-d")) {
            daemon(0, 0);
            break;
        }
    }
#endif
}