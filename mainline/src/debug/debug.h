#ifndef __DEBUG_H_
#define __DEBUG_H_

#include "common.h"
#include "string.h"

#define assert(exp) do {                                                \
        if (!exp)                                                       \
                assert_fail(#exp, __FILE__, __BASE_FILE__, __LINE__);   \
        } while (0)

#define weak_assert(exp) exp == 0 ? weak_assert_fail(#exp, __FILE__, __BASE_FILE__, __LINE__) : 0



void log_string(const char *str);
void log_hex(int data);
void log_enter(void);
void log_dec(int data);
void screen_show_char(char ch);
void log_reset(void);


int weak_assert_fail(char *exp, char *file, char *base_file, int line);
void assert_fail(char *exp, char *file, char *base_file, int line);
#endif
