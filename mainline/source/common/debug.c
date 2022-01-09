#include "debug.h"
#include "string.h"
#include "tty.h"

int weak_assert_fail(char *exp, char *file, char *base_file, int line)
{
        char buf[ASSERT_MESSAGE_LENGTH] = {0};
        uint32_t len = 0;

        vsprint(buf, "Weak assert fail: (%s) in %s, line %d", exp, file, line);
        len = strlen(buf);
        len = len > ASSERT_MESSAGE_LENGTH ? ASSERT_MESSAGE_LENGTH : len;

        tty_display(-1, len, buf, -1);

        return -1;
}

void assert_fail(char *exp, char *file, char *base_file, int line)
{
        char buf[ASSERT_MESSAGE_LENGTH] = {0};
        uint32_t len = 0;

        vsprint(buf, "Assert fail: (%s) in %s, line %d", exp, file, line);
        len = strlen(buf);
        len = len > ASSERT_MESSAGE_LENGTH ? ASSERT_MESSAGE_LENGTH : len;
        tty_display(-1, len, buf, -1);

        __asm__ __volatile__("ud2":::);
}
