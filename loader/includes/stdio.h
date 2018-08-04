
#ifndef _STDIO_H
#define _STDIO_H


#pragma pack(push, 1)


#pragma pack(pop)


typedef char *va_list;

bit16u vsprintf(const char *, const char *, va_list);
bit16u sprintf(char *, const char *, ...);

bit16u printf(const char *, ...);
int putchar(const int);
int puts(const char *);

void set_vector(const int, const bit32u);



#endif  // _STDIO_H
