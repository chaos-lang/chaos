#include "parser.h"

int initParser(int argc, char** argv) {
    ast_debug_enabled = false;

#if !defined(__clang__) || !(defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__))
    char ch;
    while ((ch = getopt_long(argc, argv, "hvd", long_options, NULL)) != -1)
    {
        switch (ch)
        {
            case 'h':
                print_help();
                exit(0);
            case 'v':
                printf("%s\n", __KAOS_LANGUAGE_VERSION__);
                exit(0);
            case 'd':
                ast_debug_enabled = true;
                break;
            case '?':
                printf("Unknown option `-%c'.\n", optopt);
                print_help();
                break;
        }
    }
#endif

    char *program_file = NULL;

    if (argc == 3) {
        program_file = argv[2];
        fp = fopen (program_file, "r");
    } else if (argc == 2) {
        if (strcmp(argv[1], "-d") == 0) {
            fp = stdin;
        } else {
            program_file = argv[1];
            fp = fopen (program_file, "r");
        }
    } else if (argc == 1) {
        fp = stdin;
    }
    fp_opened = true;

    is_interactive = (fp != stdin) ? false : true;

    if (!is_interactive) {
        program_file_path = malloc(strlen(program_file) + 1);
        strcpy(program_file_path, program_file);

        if (!is_file_exists(program_file_path)) {
            initMainFunction();
            is_interactive = false;
            fp_opened = false;
            throw_error(E_PROGRAM_FILE_DOES_NOT_EXISTS_ON_PATH, program_file_path);
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
        #if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
        using_history();
        read_history(NULL);
        #endif
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
                absorbError();
            }
        }

        #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        !is_interactive ?: printf("%s ", __KAOS_SHELL_INDICATOR__);
        #endif
        main_interpreted_module = malloc(1 + strlen(module_path_stack.arr[module_path_stack.size - 1]));
        strcpy(main_interpreted_module, module_path_stack.arr[module_path_stack.size - 1]);
        yyparse();
        if (!is_interactive)
            interpret(main_interpreted_module, INIT_PREPARSE, false);
        if (!is_interactive) break;
    } while(!feof(yyin));

    freeEverything();

    return 0;
}

void freeEverything() {
    free(main_function);
    free(scopeless);
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

    yylex_destroy();

    if (!is_interactive) {
        free(program_code);
        if (fp_opened)
            fclose(fp);
        free(program_file_path);
        free(program_file_dir);
    } else {
        #if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
        clear_history();
        for (unsigned long long i = __KAOS_LANGUAGE_KEYWORD_COUNT__; i < suggestions_length; i++) {
            free(suggestions[i]);
        }
        #endif
    }

    free_node(ast_root_node);
    free(main_interpreted_module);

    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
}

void yyerror(const char* s) {
    if (phase == PREPARSE) return;

    yyerror_msg(capitalize(s), getCurrentModule(), yytext);

    if (is_interactive) {
        loop_mode = NULL;
        function_mode = NULL;
        if (isComplexMode()) {
            freeComplexModeStack();
        }
        #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        printf("%s ", __KAOS_SHELL_INDICATOR__);
        #endif
        flushLexer();
        phase = INIT_PROGRAM;
        yyrestart_interactive();
        freeModulePathStack();
        initMainContext();
        reset_line_no_to = 0;
        yyparse();
    } else {
        freeEverything();
        exit(E_SYNTAX_ERROR);
    }
}

void absorbError() {
    ast_interactive_cursor = ast_node_cursor;

    phase = INIT_PROGRAM;
    disable_complex_mode = false;
    freeComplexModeStack();
    freeLeftRightBracketStackSymbols();
    resetFunctionParametersMode();

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[1;44m");
    #endif
    printf("%-*s", InteractiveShellErrorAbsorber_ws_col, "    Absorbed by Interactive Shell");
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");

    if (loop_execution_mode) longjmp(InteractiveShellLoopErrorAbsorber, 1);
    if (function_call_stack.size > 0) longjmp(InteractiveShellFunctionErrorAbsorber, 1);

    if (main_interpreted_module != NULL) {
        free(main_interpreted_module);
        main_interpreted_module = NULL;
    }
}
