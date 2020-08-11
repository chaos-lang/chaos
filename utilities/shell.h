#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "language.h"
#include "helpers.h"
#include "function.h"

int up_arrow_key_pressed(int count, int key);
int down_arrow_key_pressed(int count, int key);
int esc_key_pressed(int count, int key);
int ctrl_d_key_pressed();
int tab_key_pressed(int count, int key);

int shell_readline(char *buf);

char **suggestion_completion(const char *, int, int);
char *suggestion_generator(const char *, int);
int add_suggestion(char *suggestion);
unsigned long long suggestions_length;
char *suggestions[1000];

#endif
