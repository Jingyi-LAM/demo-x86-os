#ifndef __STRING_H_
#define __STRING_H_

int strlen(const char *src);
int strcmp(unsigned char *base, unsigned char *obj, unsigned int limit);
int strcpy(char* dest, char* src);
int vsprint(char *dest, const char* fmt, ...);

#endif
