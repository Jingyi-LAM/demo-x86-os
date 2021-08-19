#ifndef __DEBUG_H_
#define __DEBUG_H_

#include "typedef.h"

#define assert(exp) do {                                                \
        if (!exp)                                                       \
                assert_fail(#exp, __FILE__, __BASE_FILE__, __LINE__);   \
        } while (0)
#define weak_assert(exp) exp == 0 ? weak_assert_fail(#exp, __FILE__, __BASE_FILE__, __LINE__) : 0
#define ASSERT_MESSAGE_LENGTH   128

int weak_assert_fail(char *exp, char *file, char *base_file, int line);
void assert_fail(char *exp, char *file, char *base_file, int line);
#endif
