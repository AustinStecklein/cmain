#ifndef DEBUG_H
#define DEBUG_H

// in the future this should be going to stderr instead of stdout
#ifdef DEBUG
#include <stdio.h>
#define DEBUG_PRINT(message)                                                   \
    (printf("%s Line:%d - \e[1;32mInfo\e[0m: %s\n", __FILE__, __LINE__,        \
            message))
#define DEBUG_ERROR(message)                                                   \
    (fprintf(stderr, "%s Line:%d - \e[1;31mError\e[0m: %s\n", __FILE__,        \
             __LINE__, message))
#else
#define DEBUG_PRINT(message)
#define DEBUG_ERROR(message)
#endif

#endif
