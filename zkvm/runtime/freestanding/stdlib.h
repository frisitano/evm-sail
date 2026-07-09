/* Freestanding <stdlib.h> shim for the zkVM guest. */
#ifndef ZKVM_FREESTANDING_STDLIB_H
#define ZKVM_FREESTANDING_STDLIB_H

#include <stddef.h>

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void  free(void *ptr);

void  abort(void) __attribute__((noreturn));
void  exit(int status) __attribute__((noreturn));

void  qsort(void *base, size_t nmemb, size_t size,
            int (*compar)(const void *, const void *));

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#endif /* ZKVM_FREESTANDING_STDLIB_H */
