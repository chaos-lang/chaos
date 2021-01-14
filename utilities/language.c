/*
 * Description: Language module of the Chaos Programming Language's source
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

#include "language.h"
#include "helpers.h"

extern int kaos_lineno;

void yyerror_msg(char* error_name, char* current_module, char* cause) {
    char error_name_msg[__KAOS_MSG_LINE_LENGTH__];
    char current_module_msg[__KAOS_MSG_LINE_LENGTH__];
    char line_no_msg[__KAOS_MSG_LINE_LENGTH__];
    char cause_msg[__KAOS_MSG_LINE_LENGTH__];

    sprintf(error_name_msg, "  %s:", error_name);
    sprintf(current_module_msg, "    File: %s", current_module);
    sprintf(line_no_msg, "    Line: %d", kaos_lineno);
    sprintf(cause_msg, "    Cause: %s", cause);

    char* new_cause_msg = str_replace(cause_msg, "\n", "\\n");

    int cols[4];
    cols[0] = (int) strlen(error_name_msg) + 1;
    cols[1] = (int) strlen(current_module_msg) + 1;
    cols[2] = (int) strlen(line_no_msg) + 1;
    cols[3] = (int) strlen(new_cause_msg) + 1;
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
    fprintf(stderr, "%-*s", ws_col, current_module_msg);
#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0m");
#endif
    fprintf(stderr, "\n");

#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0;46m");
#endif
    fprintf(stderr, "%-*s", ws_col, line_no_msg);
#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0m");
#endif
    fprintf(stderr, "\n");

#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0;46m");
#endif
    fprintf(stderr, "%-*s", ws_col, new_cause_msg);
#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0m");
#endif
    fprintf(stderr, "\n");
    fflush(stderr);
    free(new_cause_msg);
}
