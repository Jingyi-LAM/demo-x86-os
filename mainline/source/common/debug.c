#include "debug.h"
#include "interrupt.h"
#include "string.h"

int weak_assert_fail(char *exp, char *file, char *base_file, int line)
{
        char buf[ASSERT_MESSAGE_LENGTH] = {0};

        vsprint(buf, "weak assert fail: (%s) in %s, line %d", exp, file, line);
        __asm__ __volatile__(
                "movl %0,       %%eax   \n\t"
                "movl $-1,      %%ebx   \n\t"
                "movl %1,       %%ecx   \n\t"
                "movl $-1,      %%edx   \n\t"
                "movl %2,       %%edi   \n\t"
                "int  $100              \n\t"
                :
                :"g"(buf), "g"(ASSERT_MESSAGE_LENGTH), "g"(SYSCALL_TTY_WRITE)
                :"eax", "ebx", "ecx", "edx", "edi"
        );

        return -1;
}

void assert_fail(char *exp, char *file, char *base_file, int line)
{
        char buf[ASSERT_MESSAGE_LENGTH] = {0};

        vsprint(buf, "assert fail: (%s) in %s, line %d", exp, file, line);
        __asm__ __volatile__(
                "movl %0,       %%eax   \n\t"
                "movl $-1,      %%ebx   \n\t"
                "movl %1,       %%ecx   \n\t"
                "movl $-1,      %%edx   \n\t"
                "movl %2,       %%edi   \n\t"
                "int  $100              \n\t"
                :
                :"g"(buf), "g"(ASSERT_MESSAGE_LENGTH), "g"(SYSCALL_TTY_WRITE)
                :"eax", "ebx", "ecx", "edx", "edi"
        );
        __asm__ __volatile__("ud2":::);
}
