#ifndef ERRORS_H
#define ERRORS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <string.h>

extern bool is_interactive;
extern jmp_buf InteractiveShellErrorAbsorber;

void throw_error(int code, char *subject);

#endif
