#include "shell.h"

int up_arrow_key_pressed(int count, int key) {
    rl_replace_line("", 0);
    return 0;
}

int down_arrow_key_pressed(int count, int key) {
    rl_replace_line("", 0);
    return 0;
}

int esc_key_pressed(int count, int key) {
    rl_replace_line("exit", 0);
    rl_redisplay();
    printf("\n%s\n", __BYE_BYE__);
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
    rl_command_func_t tab_key_pressed;
    rl_bind_keyseq("\\e[A", up_arrow_key_pressed);
    rl_bind_keyseq("\\e[B", down_arrow_key_pressed);
    rl_bind_key(27, esc_key_pressed); /* ascii code for ESC */
    rl_bind_key ('\t', tab_key_pressed);

    char *line;

    if (loop_mode || function_mode) {
        line = readline(__SHELL_INDICATOR_BLOCK__);
    } else {
        line = readline(__SHELL_INDICATOR__);
    }

    sprintf(buf,"%s\n",line);
    add_history(line);
    free(line);
    return strlen(buf);
}
