/*
 * Description: Shell module of the Chaos Programming Language's source
 *
 * Copyright (c) 2019-2020 Chaos Language Development Authority <info@chaos-lang.org>
 *
 * License: GNU General Public License v3.0
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>
 *
 * Authors: M. Mert Yildiran <me@mertyildiran.com>
 */

#ifndef KAOS_SHELL_H
#define KAOS_SHELL_H

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "language.h"
#include "helpers.h"
#include "../interpreter/function.h"

int up_arrow_key_pressed(int count, int key);
int down_arrow_key_pressed(int count, int key);
int esc_key_pressed(int count, int key);
int ctrl_d_key_pressed();
int tab_key_pressed(int count, int key);

int shell_readline(char *buf);

char **suggestion_completion(const char *, int, int);
char *suggestion_generator(const char *, int);
int add_suggestion(char *suggestion);

#endif
