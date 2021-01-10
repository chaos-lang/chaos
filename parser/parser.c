/*
 * Description: Enterance functions for the parser of the Chaos Programming Language's source
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

#include "parser.h"

extern int reset_line_no_to;
extern bool disable_complex_mode;
extern char *suggestions[1000];
extern unsigned long long suggestions_length;

bool global_unsafe = false;

#ifndef CHAOS_COMPILER
static struct option long_options[] =
{
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'v'},
    {"license", no_argument, NULL, 'l'},
    {"debug", no_argument, NULL, 'd'},
    {"compile", required_argument, NULL, 'c'},
    {"output", required_argument, NULL, 'o'},
    {"extra", required_argument, NULL, 'e'},
    {"keep", no_argument, NULL, 'k'},
    {"unsafe", no_argument, NULL, 'u'},
    {NULL, 0, NULL, 0}
};

int initParser(int argc, char** argv) {
    debug_enabled = false;
    bool compiler_mode = false;
    bool compiler_fopen_fail = false;
    char *program_file = NULL;
    char *bin_file = NULL;
    bool keep = false;
    bool unsafe = false;
    char *extra_flags = NULL;

    char opt;
    while ((opt = getopt_long(argc, argv, "hvldc:o:e:ku", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'h':
                print_help();
                exit(0);
            case 'v':
                printf("%d.%d.%d\n", __KAOS_VERSION_MAJOR__, __KAOS_VERSION_MINOR__, __KAOS_VERSION_PATCHLEVEL__);
                exit(0);
            case 'l':
                print_license();
                exit(0);
            case 'd':
                debug_enabled = true;
                break;
            case 'c':
                compiler_mode = true;
                program_file = optarg;
                fp = fopen(program_file, "r");
                if (fp == NULL)
                    compiler_fopen_fail = true;
                break;
            case 'o':
                bin_file = optarg;
                break;
            case 'e':
                extra_flags = optarg;
                break;
            case 'k':
                keep = true;
                break;
            case 'u':
                unsafe = true;
                global_unsafe = true;
                break;
            case '?':
                switch (optopt)
                {
                    case 'c':
                        throwCompilerInteractiveError();
                        break;
                    case 'o':
                        throwMissingOutputName();
                        break;
                    case 'e':
                        throwMissingExtraFlags();
                        break;
                    default:
                        print_help();
                        exit(E_INVALID_OPTION);
                        break;
                }
                break;
        }
    }

    if (bin_file != NULL && !compiler_mode)
        throwMissingCompileOption();

    if (fp == NULL) {
        if (argc == 1) {
            fp = stdin;
        } else if (debug_enabled && argc == 2) {
            fp = stdin;
        } else if (!compiler_fopen_fail) {
            program_file = argv[argc - 1];
            fp = fopen (program_file, "r");
        }
    }

    fp_opened = true;

    is_interactive = (fp != stdin) ? false : true;

    if (!is_interactive) {
        program_file_path = malloc(strlen(program_file) + 1);
        strcpy(program_file_path, program_file);

        if (!is_file_exists(program_file_path)) {
            program_file_path = realloc(program_file_path, strlen("N/A") + 1);
            strcpy(program_file_path, "N/A");
            initMainFunction();
            is_interactive = false;
            fp_opened = false;
            throw_error(E_PROGRAM_FILE_DOES_NOT_EXISTS_ON_PATH, program_file);
        }

        program_file_dir = malloc(strlen(program_file_path) + 1);
        strcpy(program_file_dir, program_file_path);
        char *ptr = strrchr(program_file_dir, __KAOS_PATH_SEPARATOR_ASCII__);
        if (ptr) {
            *ptr = '\0';
        }
    } else {
        char buff[PATH_MAX];
        GetCurrentDir(buff, PATH_MAX);

        program_file_dir = malloc(strlen(buff) + 1);
        strcpy(program_file_dir, buff);

        program_file_path = strcat_ext(program_file_dir, __KAOS_PATH_SEPARATOR__);
        program_file_path = strcat_ext(program_file_path, __KAOS_INTERACTIVE_MODULE_NAME__);
    }

    yyin = fp;

    if (is_interactive) {
#   if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
        using_history();
        read_history(NULL);
#   endif
        greet();
        phase = INIT_PROGRAM;
    } else {
        program_code = fileGetContents(program_file_path);
        size_t program_length = strlen(program_code);
        program_code = (char*)realloc(program_code, program_length + 2);
        program_code[program_length] = '\n';
        program_code[program_length + 1] = '\0';
        switchBuffer(program_code, INIT_PROGRAM);
    }

    initMainFunction();

    main_interpreted_module = NULL;

    do {
        if (is_interactive) {
            if (setjmp(InteractiveShellErrorAbsorber)) {
            }
        }

#   if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        !is_interactive ? is_interactive : printf("%s ", __KAOS_SHELL_INDICATOR__);
#   endif
        main_interpreted_module = malloc(1 + strlen(module_path_stack.arr[module_path_stack.size - 1]));
        strcpy(main_interpreted_module, module_path_stack.arr[module_path_stack.size - 1]);
        yyparse();
        if (!is_interactive) {
            if (compiler_mode) {
                compile(main_interpreted_module, INIT_PREPARSE, bin_file, extra_flags, keep, unsafe);
            } else {
                interpret(main_interpreted_module, INIT_PREPARSE, false, unsafe);
            }
        }
        if (!is_interactive) break;
    } while(!feof(yyin));

    freeEverything();

    return 0;
}
#endif

void freeEverything() {
    free(scopeless->function);
    free(scopeless);
    free(dummy_scope);
    freeAllSymbols();
    freeAllFunctions();
    freeModulesBuffer();
    freeFunctionNamesBuffer();
    free(function_names_buffer.arr);
    if (strlen(decision_buffer) > 0) free(decision_buffer);
    freeModulePathStack();
    freeModuleStack();
    freeComplexModeStack();
    freeLeftRightBracketStack();
    freeFreeStringStack();
    freeNestedComplexModeStack();
    free(function_call_stack.arr);

#ifndef CHAOS_COMPILER
    yylex_destroy();

    if (!is_interactive) {
        free(program_code);
        if (fp_opened)
            fclose(fp);
        free(program_file_path);
        free(program_file_dir);
    } else {
#   if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
        clear_history();
        for (unsigned long long i = __KAOS_LANGUAGE_KEYWORD_COUNT__; i < suggestions_length; i++) {
            free(suggestions[i]);
        }
#   endif
    }

    free_node(ast_root_node);
    free(main_interpreted_module);

    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
#endif
}

void yyerror(const char* s) {
    if (phase == PREPARSE) return;

    yyerror_msg(capitalize(s), getCurrentModule(), yytext);

#ifndef CHAOS_COMPILER
    if (is_interactive) {
        loop_mode = NULL;
        function_mode = NULL;
        if (isComplexMode()) {
            freeComplexModeStack();
        }
#   if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        printf("%s ", __KAOS_SHELL_INDICATOR__);
#   endif
        flushLexer();
        phase = INIT_PROGRAM;
        yyrestart_interactive();
        freeModulePathStack();
        initMainContext();
        reset_line_no_to = 0;
        yyparse();
    } else {
#endif
        freeEverything();
        exit(E_SYNTAX_ERROR);
#ifndef CHAOS_COMPILER
    }
#endif
}

#ifndef CHAOS_COMPILER
void absorbError() {
    ast_interactive_cursor = ast_node_cursor;

    phase = INIT_PROGRAM;
    disable_complex_mode = false;
    preemptive_freeAllSymbols();
    freeComplexModeStack();
    freeLeftRightBracketStackSymbols();
    resetFunctionParametersMode();
    freeFunctionNamesBuffer();

#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    printf("\033[1;44m");
#   endif
    printf("%-*s", InteractiveShellErrorAbsorber_ws_col, "    Absorbed by Interactive Shell");
#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    printf("\033[0m");
#   endif
    printf("\n");

    if (loop_execution_mode) longjmp(InteractiveShellLoopErrorAbsorber, 1);
    if (function_call_stack.size > 0) longjmp(InteractiveShellFunctionErrorAbsorber, 1);

    if (main_interpreted_module != NULL) {
        free(main_interpreted_module);
        main_interpreted_module = NULL;
    }

    longjmp(InteractiveShellErrorAbsorber, 1);
}

void throwCompilerInteractiveError() {
    printf("Compile option '-c' cannot be used with the interactive mode.\n\n");
    printf("Specify a Chaos program file with: ");
#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    printf("\033[1;45m");
#   endif
    printf(" chaos -c hello.kaos ");
#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    printf("\033[0m");
#   endif
    printf("\n\n");
    print_help();
    exit(E_INVALID_OPTION);
}

void throwMissingOutputName() {
    printf("You have to supply an output filename while using the option '-o'.\n\n");
    printf("Correct command should look like this: ");
#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    printf("\033[1;45m");
#   endif

#   if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    printf(" chaos -c hello.kaos -o hello.exe ");
#   else
    printf(" chaos -c hello.kaos -o hello ");
#   endif

#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    printf("\033[0m");
#   endif
    printf("\n\n");
    print_help();
    exit(E_INVALID_OPTION);
}

void throwMissingCompileOption() {
    printf("You have to give the path of Chaos program file with the option '-c'.\n\n");
    printf("Correct command should look like this: ");
#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    printf("\033[1;45m");
#   endif

#   if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    printf(" chaos -c hello.kaos -o hello.exe ");
#   else
    printf(" chaos -c hello.kaos -o hello ");
#   endif

#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    printf("\033[0m");
#   endif
    printf("\n\n");
    print_help();
    exit(E_INVALID_OPTION);
}

void throwMissingExtraFlags() {
    printf("You have to specify a string that contains the extra flags with the option '-e'.\n\n");
    printf("Correct command should look like this: ");
#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    printf("\033[1;45m");
#   endif

#   if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    printf(" chaos -c hello.kaos -o hello.exe -e \"-ggdb\" ");
#   else
    printf(" chaos -c hello.kaos -o hello -e \"-ggdb\" ");
#   endif

#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    printf("\033[0m");
#   endif
    printf("\n\n");
    print_help();
    exit(E_INVALID_OPTION);
}
#endif
