#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct Symbol Symbol;
enum Type { BOOL, NUMBER, STRING, ANY, ARRAY, DICT, VOID };
enum ValueType { V_BOOL, V_INT, V_FLOAT, V_STRING, V_VOID };
enum Role { DEFAULT, PARAM, CALL_PARAM };

#include "errors.h"
#include "utilities/helpers.h"
#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
#include "utilities/shell.h"
#endif
#include "functions/function.h"

typedef struct Symbol {
    char *name;
    char *secondary_name;
    enum Type type;
    enum Type secondary_type;
    union Value {
        bool b;
        int i;
        char *s;
        float f;
    } value;
    enum ValueType value_type;
    struct Symbol* previous;
    struct Symbol* next;
    struct Symbol** children;
    int children_count;
    char *key;
    struct _Function* scope;
    int recursion_depth;
    enum Role role;
    struct _Function* param_of;
} Symbol;

Symbol* symbol_cursor;
Symbol* start_symbol;
Symbol* end_symbol;

Symbol* complex_mode;
int symbol_counter;

Symbol* addSymbol(char *name, enum Type type, union Value value, enum ValueType value_type);
Symbol* updateSymbol(char *name, enum Type type, union Value value, enum ValueType value_type);
void removeSymbolByName(char *name);
void removeSymbol(Symbol* symbol);
void freeSymbol(Symbol* symbol);
Symbol* getSymbol(char *name);
Symbol* deepCopySymbol(Symbol* symbol, enum Type type, char *key);
Symbol* deepCopyComplex(char *name, Symbol* symbol);
float getSymbolValueFloat(char *name);
void printSymbolValue(Symbol* symbol, bool is_complex);
void printSymbolValueEndWith(Symbol* symbol, char *end);
void printSymbolValueEndWithNewLine(Symbol* symbol);
bool isDefined(char *name);
void addSymbolToComplex(Symbol* symbol);
void printSymbolTable();
void addSymbolBool(char *name, bool b);
void updateSymbolBool(char *name, bool b);
void addSymbolInt(char *name, int i);
void updateSymbolInt(char *name, int i);
void addSymbolFloat(char *name, float f);
void updateSymbolFloat(char *name, float f);
void addSymbolString(char *name, char *s);
void updateSymbolString(char *name, char *s);
void addSymbolArray(char *name);
Symbol* createCloneFromSymbolByName(char *clone_name, enum Type type, char *name, enum Type extra_type);
Symbol* createCloneFromSymbol(char *clone_name, enum Type type, Symbol* symbol, enum Type extra_type);
Symbol* updateSymbolByClonning(char *clone_name, char *name);
bool isComplexIllegal(enum Type type);
void finishComplexMode(char *name, enum Type type);
Symbol* getArrayElement(char *name, int i);
void cloneSymbolToComplex(char *name, char *key);
void updateComplexElement(char *name, int i, char *key, enum Type type, union Value value);
void updateComplexElementBool(char* name, int index, char *key, bool b);
void updateComplexElementInt(char* name, int index, char *key, int i);
void updateComplexElementFloat(char* name, int index, char *key, float f);
void updateComplexElementString(char* name, int index, char *key, char *s);
void updateComplexElementSymbol(char* name, int index, char *key, char* source_name);
void removeComplexElement(char *name, int i, char *key);
void addSymbolDict(char *name);
void addSymbolAnyString(char *name, char *s);
void addSymbolAnyInt(char *name, int i);
void addSymbolAnyFloat(char *name, float f);
void addSymbolAnyBool(char *name, bool b);
Symbol* getDictElement(char *name, char *key);
_Function* getCurrentScope();
Symbol* getSymbolFunctionParameter(char *name);
void freeAllSymbols();
Symbol* assignByTypeCasting(Symbol* clone_symbol, Symbol* symbol);
Symbol* createSymbolWithoutValueType(char *name, enum Type type);
void removeSymbolsByScope(_Function* scope);

#endif
