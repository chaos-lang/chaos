#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
#include "shell.h"

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
    "from"
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
    //rl_command_func_t tab_key_pressed;
    rl_bind_keyseq("\\e[A", up_arrow_key_pressed);
    rl_bind_keyseq("\\e[B", down_arrow_key_pressed);
    rl_bind_key(27, esc_key_pressed); /* ascii code for ESC */
    //rl_bind_key ('\t', tab_key_pressed);
    rl_attempted_completion_function = suggestion_completion;

    char *line;

    if (loop_mode || function_mode || isComplexMode() || decision_mode) {
        line = readline(__KAOS_SHELL_INDICATOR_BLOCK__);
    } else {
        line = readline(__KAOS_SHELL_INDICATOR__);
    }

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
#endif
