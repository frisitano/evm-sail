/* Freestanding <stdio.h> shim for the zkVM guest. Decouples sail.c / mini-gmp
 * from newlib's hosted stdio. Only the symbols actually referenced are declared;
 * their definitions live in runtime.c and route to the HTIF console. */
#ifndef ZKVM_FREESTANDING_STDIO_H
#define ZKVM_FREESTANDING_STDIO_H

#include <stddef.h>
#include <stdarg.h>

typedef struct __zkvm_FILE FILE;

extern FILE *stdout;
extern FILE *stderr;
extern FILE *stdin;

int  printf(const char *fmt, ...);
int  fprintf(FILE *stream, const char *fmt, ...);
int  sprintf(char *str, const char *fmt, ...);
int  snprintf(char *str, size_t size, const char *fmt, ...);
int  vsnprintf(char *str, size_t size, const char *fmt, va_list ap);
int  asprintf(char **strp, const char *fmt, ...);
int  fputs(const char *s, FILE *stream);
int  fputc(int c, FILE *stream);
int  putchar(int c);
int  puts(const char *s);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int  fflush(FILE *stream);

#endif /* ZKVM_FREESTANDING_STDIO_H */
