#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdio.h>
#include <string.h>

typedef struct _Function _Function;

#include "../symbol.h"
#include "../errors.h"
#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
#include "../utilities/shell.h"
#endif
#include "../utilities/language.h"
#include "../utilities/phase.h"

extern enum Phase phase;
enum BlockType { B_EXPRESSION, B_FUNCTION };

typedef struct _Function {
    char *name;
    int line_no;
    struct Symbol** parameters;
    int parameter_count;
    enum Type type;
    struct Symbol* symbol;
    struct _Function* previous;
    struct _Function* next;
    struct _Function* parent_scope;
    string_array decision_expressions;
    string_array decision_functions;
    char *decision_default;
    char *context;
    char *module_context;
    char *module;
    char *body;
} _Function;

_Function* function_cursor;
_Function* start_function;
_Function* end_function;

_Function* function_mode;

_Function* function_parameters_mode;

_Function* executed_function;

_Function* main_function;
_Function* scopeless;

_Function* scope_override;

_Function* decision_mode;
_Function* decision_expression_mode;
_Function* decision_function_mode;
Symbol* decision_symbol_chain;
char *decision_buffer;

string_array modules_buffer;

string_array function_names_buffer;

int recursion_depth;

extern char *program_file_path;
extern char *program_file_dir;
string_array module_path_stack;
string_array module_stack;

extern int module_parsing;

int reset_line_no_to;

void startFunction(char *name, enum Type type);
void endFunction();
void freeFunctionMode();
_Function* getFunction(char *name, char *module);
void printFunctionTable();
void callFunction(char *name, char *module);
void startFunctionParameters();
void addFunctionParameter(char *secondary_name, enum Type type);
void addSymbolToFunctionParameters(Symbol* symbol);
void addFunctionCallParameterBool(bool b);
void addFunctionCallParameterInt(int i);
void addFunctionCallParameterFloat(float f);
void addFunctionCallParameterString(char *s);
void addFunctionCallParameterSymbol(char *name);
void returnSymbol(char *name);
void printFunctionReturn(char *name, char *module);
void initMainFunction();
void initScopeless();
void initMainContext();
void freeFunction(_Function* function);
void freeAllFunctions();
bool block(enum BlockType type);
void addBooleanDecision();
void addDefaultDecision();
void executeDecision(_Function* function);
void appendModuleToModuleBuffer(char *name);
void prependModuleToModuleBuffer(char *name);
void addFunctionNameToFunctionNamesBuffer(char *name);
void handleModuleImport(char *module_name, bool directly_import);
void freeModulesBuffer();
void freeFunctionNamesBuffer();
bool isInFunctionNamesBuffer(char *name);
void popModuleStack();
void pushModuleStack(char *module_path, char *module);
void freeModulePathStack();
void freeModuleStack();
char* getCurrentModule();

#endif
