#ifndef __STRING_H_
#define __STRING_H_

int strlen(const char *src);
int strcmp(unsigned char *base, unsigned char *obj, unsigned int limit);
int strcpy(char* dest, char* src);
int hex2string(char *dest, int data, char is_need_fill);
int dec2string(char *dest, int data);
int vsprint(char *dest, const char* fmt, ...);

#endif
