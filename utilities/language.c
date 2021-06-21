/*
 * Description: Language module of the Chaos Programming Language's source
 *
 * Copyright (c) 2019-2021 Chaos Language Development Authority <info@chaos-lang.org>
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

#include "language.h"
#include "helpers.h"

extern int yylineno;
extern FILE* tmp_stdin;

void yyerror_msg(char* error_name, char* current_module, char* cause) {
    char error_name_msg[__KAOS_MSG_LINE_LENGTH__];
    char info[__KAOS_MSG_LINE_LENGTH__];
    char line_msg[__KAOS_MSG_LINE_LENGTH__];
    int indent = 2;

    sprintf(error_name_msg, "%*c%s:", indent, ' ', error_name);
    sprintf(info, "%*cFile: \"%s\", line %d, cause: %s", indent * 2, ' ', current_module, yylineno, cause);
    char* info_msg = str_replace(info, "\n", "\\n");

    FILE* fp_module = NULL;
#ifndef CHAOS_COMPILER
    if (is_interactive) {
        fseek(tmp_stdin, 0, SEEK_SET);
        fp_module = tmp_stdin;
    } else {
#endif
        fp_module = fopen(current_module, "r");
#ifndef CHAOS_COMPILER
    }
#endif
    char *line = NULL;
    if (fp_module == NULL) {
        line = malloc(4);
        strcpy(line, "???");
    } else {
        line = get_nth_line(fp_module, yylineno);
#ifndef CHAOS_COMPILER
        if (fp_module != tmp_stdin)
#endif
            fclose(fp_module);
        if (line == NULL) {
            line = malloc(4);
            strcpy(line, "???");
        }
    }
    sprintf(
        line_msg,
        "%*c%s",
        indent * 3,
        ' ',
        trim(line)
    );
    free(line);

    int cols[3];
    cols[0] = (int) strlen(error_name_msg) + 1;
    cols[1] = (int) strlen(info_msg) + 1;
    cols[2] = (int) strlen(line_msg) + 1;
    int ws_col = largest(cols, 3) + 4;

    fflush(stdout);
#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[1;46m");
#endif
    fprintf(stderr, "%-*s", ws_col, error_name_msg);
#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0m");
#endif
    fprintf(stderr, "\n");

#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0;46m");
#endif
    fprintf(stderr, "%-*s", ws_col, info_msg);
#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0m");
#endif
    fprintf(stderr, "\n");
    free(info_msg);

#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0;46m");
#endif
    fprintf(stderr, "%-*s", ws_col, line_msg);
#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0m");
#endif
    fprintf(stderr, "\n");
    fflush(stderr);
}
