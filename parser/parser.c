/*
 * Description: Enterance functions for the parser of the Chaos Programming Language's source
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

#include "parser.h"

extern bool disable_complex_mode;
extern char *suggestions[1000];
extern unsigned long long suggestions_length;

bool compiling_a_function = false;

#ifndef CHAOS_COMPILER
static struct option long_options[] =
{
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'v'},
    {"license", no_argument, NULL, 'l'},
    {"debug", required_argument, NULL, 'd'},
    {"compile", required_argument, NULL, 'c'},
    {"output", required_argument, NULL, 'o'},
    {"extra", required_argument, NULL, 'e'},
    {"keep", no_argument, NULL, 'k'},
    {"ast", no_argument, NULL, 'a'},
    {NULL, 0, NULL, 0}
};

int initParser(int argc, char** argv) {
    unsigned short debug_level = 0;
    bool compiler_mode = false;
    bool compiler_fopen_fail = false;
    bool print_ast = false;
    char *program_file = NULL;
    char *bin_file = NULL;
    // bool keep = false;
    // char *extra_flags = NULL;

    char opt;
    while ((opt = getopt_long(argc, argv, "hvld:c:o:e:k:a:", long_options, NULL)) != -1)
    {
        switch (opt) {
        case 'h':
            print_help();
            exit(0);
        case 'v':
            printf("%d.%d.%d\n", __KAOS_VERSION_MAJOR__, __KAOS_VERSION_MINOR__, __KAOS_VERSION_PATCHLEVEL__);
            exit(0);
        case 'l':
            print_license();
            exit(0);
        case 'd': {
            debug_level = atoi(optarg);
            break;
        }
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
            // extra_flags = optarg;
            break;
        case 'k':
            // keep = true;
            break;
        case 'a':
            print_ast = true;
            break;
        case '?':
            switch (optopt) {
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
        } else if (debug_level > 0 && argc == 3) {
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
        tmp_stdin = tmpfile();

        char buff[PATH_MAX];
        if (GetCurrentDir(buff, PATH_MAX) == NULL) {
            fprintf(stderr, "getcwd() error\n");
            exit(1);
        }

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
        interactive_program = initProgram();
        interactive_c = new_cpu(interactive_program, debug_level);
        initCallJumps();
    } else {
        program_code = fileGetContents(program_file_path);
        size_t program_length = strlen(program_code);
        program_code = (char*)realloc(program_code, program_length + 2);
        program_code[program_length] = '\n';
        program_code[program_length + 1] = '\0';
        switchBuffer(program_code, INIT_PROGRAM);
    }

    initASTRoot();
    initMainFunction();

    main_interpreted_module = NULL;
    prev_stmt_count = 0;
    prev_import_count = 0;

    do {
        if (is_interactive) {
            if (setjmp(InteractiveShellErrorAbsorber)) {
            }
        }

#   if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        !is_interactive ? is_interactive : printf("%s ", __KAOS_SHELL_INDICATOR__);
#   endif
        main_interpreted_module = malloc(1 + strlen(_ast_root->files[_ast_root->file_count - 1]->module_path));
        strcpy(main_interpreted_module, _ast_root->files[_ast_root->file_count - 1]->module_path);
        yyparse();

        if (is_interactive)
            break;

        if (print_ast || debug_level > 0) {
            if (!print_ast)
                printf("Abstract Syntax Tree (AST):\n");
            printAST(_ast_root);
            if (debug_level == 1 || print_ast)
                exit(0);
        }

        KaosIR* program = compile(_ast_root);

        if (debug_level > 1) {
            printf("\nJIT Abstraction Layer:\n");
            emit(program);
            if (debug_level == 2)
                exit(0);
        }

        if (debug_level > 2)
            printf("\nJIT Runtime:\n");

        cpu *c = new_cpu(program, debug_level);
        run_cpu(c);
        free_cpu(c);
        // if (!is_interactive) {
        //     if (compiler_mode) {
        //         compile(main_interpreted_module, INIT_PREPARSE, bin_file, extra_flags, keep);
        //     } else {
        //         interpret(main_interpreted_module, INIT_PREPARSE, false);
        //     }
        // }
        if (!is_interactive) break;
    } while(!feof(yyin));

    freeEverything();

    return 0;
}
#endif

void compile_interactive()
{
    bool new_imports = _ast_root->files[0]->imports->spec_count > prev_import_count;
    bool new_stmts = _ast_root->files[0]->stmt_list->stmt_count > prev_stmt_count;
    if (!new_imports && !new_stmts)
        return;
    prev_import_count = _ast_root->files[0]->imports->spec_count;
    prev_stmt_count = _ast_root->files[0]->stmt_list->stmt_count;
    turnLastExprStmtIntoPrintStmt();
    if (interactive_c->debug_level > 0) {
        printf("Abstract Syntax Tree (AST):\n");
        printAST(_ast_root);
    }

    if (new_imports) {
        compileImports(_ast_root, interactive_program);
        _ast_root->files[0]->imports_handled = false;
        declare_functions(_ast_root, interactive_program);
        compiling_a_function = true;
        compile_functions(_ast_root, interactive_program);
        compiling_a_function = false;
        current_file_index = 0;

        push_inst_(interactive_program, HLT);
        interactive_program->hlt_count++;
        interactive_c->ic = interactive_program->size - 1;
        if (interactive_c->debug_level > 1) {
            printf("\nJIT Abstraction Layer:\n");
            emit(interactive_program);
        }

        return;
    }

    bool any_stmts = _ast_root->files[0]->stmt_list->stmt_count > 0;
    bool is_function = false;
    if (any_stmts) {
        Stmt* stmt = _ast_root->files[0]->stmt_list->stmts[0];
        is_function = declare_function(stmt, _ast_root->files[0], interactive_program);
        if (stmt->kind == DeclStmt_kind && stmt->v.decl_stmt->decl->kind == FuncDecl_kind)
            compiling_a_function = true;
        compileStmt(interactive_program, stmt);
        compiling_a_function = false;
        push_inst_(interactive_program, HLT);
        interactive_program->hlt_count++;
        if (!is_function)
            fillCallJumps(interactive_program);
    }

    if (interactive_c->debug_level > 1) {
        printf("\nJIT Abstraction Layer:\n");
        emit(interactive_program);
    }

    if (!any_stmts)
        return;

    interactive_c->program = interactive_program;
    if (is_function)
        interactive_c->ic = interactive_program->size - 1;
    else
        run_cpu(interactive_c);
}

void freeEverything() {
    freeAllSymbols();
    free(scopeless->function);
    free(scopeless);
    free(dummy_scope);
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
    free(program_file_path);

#ifndef CHAOS_COMPILER
    yylex_destroy();

    if (!is_interactive) {
        free(program_code);
        if (fp_opened)
            fclose(fp);
        free(program_file_dir);
    } else {
#   if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
        fclose(tmp_stdin);
        clear_history();
        for (unsigned long long i = __KAOS_LANGUAGE_KEYWORD_COUNT__; i < suggestions_length; i++) {
            free(suggestions[i]);
        }
#   endif
    }

    free(main_interpreted_module);

    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
#endif
}

void yyerror(const char* s) {
    if (phase == PREPARSE) return;

    char *error_name = capitalize(s);
    yyerror_msg(error_name, getCurrentModule(), yytext);
    free(error_name);

#ifndef CHAOS_COMPILER
    if (is_interactive) {
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
    phase = INIT_PROGRAM;
    disable_complex_mode = false;
    freeComplexModeStack();
    freeLeftRightBracketStackSymbols();
    resetFunctionParametersMode();
    freeFunctionNamesBuffer();
    scope_override = NULL;

    fflush(stdout);
#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[1;44m");
#   endif
    fprintf(stderr, "%-*s", InteractiveShellErrorAbsorber_ws_col, "  Absorbed by Interactive Shell");
#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0m");
#   endif
    fprintf(stderr, "\n");
    fflush(stderr);

    if (compiling_a_function) {
        removeFunction(end_function);
        compiling_a_function = false;
    }

    if (main_interpreted_module != NULL) {
        free(main_interpreted_module);
        main_interpreted_module = NULL;
    }

    longjmp(InteractiveShellErrorAbsorber, 1);
}

void throwCompilerInteractiveError() {
    fflush(stdout);
    fprintf(stderr, "Compile option '-c' cannot be used with the interactive mode.\n\n");
    fprintf(stderr, "Specify a Chaos program file with: ");
#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[1;45m");
#   endif
    fprintf(stderr, " chaos -c hello.kaos ");
#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0m");
#   endif
    fprintf(stderr, "\n\n");
    fflush(stderr);
    print_help();
    exit(E_INVALID_OPTION);
}

void throwMissingOutputName() {
    fflush(stdout);
    fprintf(stderr, "You have to supply an output filename while using the option '-o'.\n\n");
    fprintf(stderr, "Correct command should look like this: ");
#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[1;45m");
#   endif

#   if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    fprintf(stderr, " chaos -c hello.kaos -o hello.exe ");
#   else
    fprintf(stderr, " chaos -c hello.kaos -o hello ");
#   endif

#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0m");
#   endif
    fprintf(stderr, "\n\n");
    fflush(stderr);
    print_help();
    exit(E_INVALID_OPTION);
}

void throwMissingCompileOption() {
    fflush(stdout);
    fprintf(stderr, "You have to give the path of Chaos program file with the option '-c'.\n\n");
    fprintf(stderr, "Correct command should look like this: ");
#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[1;45m");
#   endif

#   if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    fprintf(stderr, " chaos -c hello.kaos -o hello.exe ");
#   else
    fprintf(stderr, " chaos -c hello.kaos -o hello ");
#   endif

#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0m");
#   endif
    fprintf(stderr, "\n\n");
    fflush(stderr);
    print_help();
    exit(E_INVALID_OPTION);
}

void throwMissingExtraFlags() {
    fflush(stdout);
    fprintf(stderr, "You have to specify a string that contains the extra flags with the option '-e'.\n\n");
    fprintf(stderr, "Correct command should look like this: ");
#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[1;45m");
#   endif

#   if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    fprintf(stderr, " chaos -c hello.kaos -o hello.exe -e \"-ggdb\" ");
#   else
    fprintf(stderr, " chaos -c hello.kaos -o hello -e \"-ggdb\" ");
#   endif

#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0m");
#   endif
    fprintf(stderr, "\n\n");
    fflush(stderr);
    print_help();
    exit(E_INVALID_OPTION);
}
#endif
