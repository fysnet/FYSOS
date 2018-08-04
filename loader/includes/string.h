
#ifndef STRING_H
#define STRING_H

char *strchr(char *, int);
unsigned strlen(char *);
char *strcpy(char *, const char *);
char *strncpy(char *, const char *, unsigned);
char *strcat(char *, const char *);

void *memset(void *, const bit8u, int);
void *memcpy(void *, const void *, int);
void *memmove(void *, void *, int);
int strcmp(const char *, const char *);
int stricmp(const char *, const char *);


#endif  // STRING_H
