#ifndef __STRING_H_
#define __STRING_H_

int str_len(const char *src);
int str_cmp(unsigned char *base, unsigned char *obj, unsigned int limit);
int strcpy(char* dest, char* src);
int vsprint(char *dest, const char* fmt, ...);

#endif
