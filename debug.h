#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
// The difference between the debug version and the non debug version is
// the debug version doesn't have an assert
#ifdef DEBUG
#define DEBUG_PRINT(message)                                                   \
    (printf("%s Line:%d - \e[1;32mInfo\e[0m: %s\n", __FILE__, __LINE__,        \
            message))

#define DEBUG_ERROR(message)                                                   \
    do {                                                                       \
        fprintf(stderr, "%s Line:%d - \e[1;31mError\e[0m: %s\n", __FILE__,     \
                __LINE__, message);                                            \
    } while (0)

#else
#include <assert.h>
#define DEBUG_PRINT(message)
#define DEBUG_ERROR(message)                                                   \
    do {                                                                       \
        fprintf(stderr, "%s Line:%d - \e[1;31mError\e[0m: %s\n", __FILE__,     \
                __LINE__, message);                                            \
        assert(0);                                                             \
    } while (0)

#endif
#endif
