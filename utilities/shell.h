#ifndef SHELL_H
#define SHELL_H

#include <readline/readline.h>
#include <readline/history.h>
#include "language.h"
#include "loop.h"
#include "function.h"

int up_arrow_key_pressed(int count, int key);
int down_arrow_key_pressed(int count, int key);
int esc_key_pressed(int count, int key);
int tab_key_pressed(int count, int key);

int shell_readline(char *buf);

#endif
