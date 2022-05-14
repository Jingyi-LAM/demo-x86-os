#ifndef __LOG_H_
#define __LOG_H_

#define LTRACE(x...) do { if (LOCAL_TRACE) { add_log(x); } } while (0)

void log_buffer_init(void);
void add_log(const char *fmt, ...);

#endif
