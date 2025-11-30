#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h> // NOLINT
// The difference between the debug version and the non debug version is
// the debug version doesn't have an assert
#ifdef DEBUG
#define DEBUG_PRINT(message)                                                   \
    (printf("%s Line:%d - \033[1;32mInfo\033[0m: %s\n", __FILE__, __LINE__,    \
            message))

#define DEBUG_ERROR(message)                                                   \
    do {                                                                       \
        /* NOLINTNEXTLINE */                                                   \
        fprintf(stderr, "%s Line:%d - \033[1;31mError\033[0m: %s\n", __FILE__, \
                __LINE__, message);                                            \
    } while (0)

#else
#include <assert.h>
#define DEBUG_PRINT(message)
#define DEBUG_ERROR(message)                                                   \
    do {                                                                       \
        /* NOLINTNEXTLINE */                                                   \
        fprintf(stderr, "%s Line:%d - \033[1;31mError\033[0m: %s\n", __FILE__, \
                __LINE__, message);                                            \
    } while (0)

#endif
#endif
