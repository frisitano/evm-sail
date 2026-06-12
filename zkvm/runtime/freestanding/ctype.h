/* Freestanding <ctype.h> shim for the zkVM guest. */
#ifndef ZKVM_FREESTANDING_CTYPE_H
#define ZKVM_FREESTANDING_CTYPE_H
int isdigit(int c);
int isspace(int c);
int isxdigit(int c);
int isalpha(int c);
int isupper(int c);
int islower(int c);
int toupper(int c);
int tolower(int c);
#endif
