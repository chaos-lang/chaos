/*
 * Description: Shell module of the Chaos Programming Language's source
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

#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
#   include "shell.h"

extern unsigned long long loops_inside_function_counter;
extern bool is_complex_parsing;

unsigned long long suggestions_length = __KAOS_LANGUAGE_KEYWORD_COUNT__;
char *suggestions[1000] = {
    "exit",
    "quit",
    "print",
    "true",
    "false",
    "symbol_table",
    "del",
    "return",
    "times do",
    "end",
    "foreach",
    "as",
    "bool",
    "num",
    "str",
    "list",
    "dict",
    "any",
    "void",
    "null",
    "def",
    "and",
    "or",
    "not",
    "default",
    "function_table",
    "INFINITE",
    "echo",
    "pretty",
    "import",
    "from",
    "break",
    "continue"
};

int up_arrow_key_pressed(int count, int key) {
    HIST_ENTRY *shell_history = previous_history();
    if (shell_history == NULL) return 0;
    if (strcmp(shell_history->line, rl_line_buffer) == 0) {
        up_arrow_key_pressed(0, 0);
        return 0;
    }
    rl_replace_line(shell_history->line, 0);
    rl_redisplay();
    rl_end_of_line(0, 0);
    return 0;
}

int down_arrow_key_pressed(int count, int key) {
    HIST_ENTRY *shell_history = next_history();
    if (shell_history == NULL) return 0;
    if (strcmp(shell_history->line, rl_line_buffer) == 0) {
        down_arrow_key_pressed(0, 0);
        return 0;
    }
    rl_replace_line(shell_history->line, 0);
    rl_redisplay();
    rl_end_of_line(0, 0);
    return 0;
}

int esc_key_pressed(int count, int key) {
    return ctrl_d_key_pressed();
}

int ctrl_d_key_pressed() {
    rl_replace_line("exit", 0);
    rl_redisplay();
    printf("\n");
    print_bye_bye();
    freeEverything();
    exit(0);
}

int tab_key_pressed(int count, int key) {
    rl_insert_text("    ");
    return 0;
}

int shell_readline(char *buf) {
    rl_command_func_t up_arrow_key_pressed;
    rl_command_func_t down_arrow_key_pressed;
    rl_command_func_t esc_key_pressed;
    rl_bind_keyseq("\\e[A", up_arrow_key_pressed);
    rl_bind_keyseq("\\e[B", down_arrow_key_pressed);
    rl_bind_key(27, esc_key_pressed); /* ascii code for ESC */
    rl_attempted_completion_function = suggestion_completion;

    char *line;

    if (is_interactive && (loops_inside_function_counter > 0 || is_complex_parsing)) {
        line = readline(__KAOS_SHELL_INDICATOR_BLOCK__);
    } else {
        line = readline(__KAOS_SHELL_INDICATOR__);
    }
    fprintf(tmp_stdin, "%s\n", line);

    if (line == NULL) {
        ctrl_d_key_pressed();
    }

    sprintf(buf,"%s\n",line);
    add_history(line);
    write_history(NULL);
    free(line);

    return strlen(buf);
}

char **suggestion_completion(const char *text, int start, int end)
{
    rl_attempted_completion_over = 1;
    if (strcmp(trim_string(rl_line_buffer), "") == 0) {
        tab_key_pressed(0, 0);
        return NULL;
    }
    return rl_completion_matches(text, suggestion_generator);
}

char *suggestion_generator(const char *text, int state)
{
    static int list_index, len;
    char *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while ((name = suggestions[list_index++])) {
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    return NULL;
}

int add_suggestion(char *suggestion) {
    if (!is_interactive || suggestion == NULL) {
        return 0;
    }
    suggestions[suggestions_length] = malloc(1 + strlen(suggestion));
    strcpy(suggestions[suggestions_length], suggestion);
    suggestions_length++;
    return 0;
}
#else
typedef int make_iso_compilers_happy;
#endif
