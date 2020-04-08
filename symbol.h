#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct Symbol Symbol;

#include "enums.h"
#include "errors.h"
#include "utilities/helpers.h"
#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
#include "utilities/shell.h"
#endif
#include "functions/function.h"

typedef struct Symbol {
    unsigned long long id;
    char *name;
    char *secondary_name;
    enum Type type;
    enum Type secondary_type;
    union Value {
        bool b;
        long long i;
        char *s;
        long double f;
    } value;
    short sign;
    enum ValueType value_type;
    struct Symbol* previous;
    struct Symbol* next;
    struct Symbol** children;
    unsigned long children_count;
    char *key;
    struct _Function* scope;
    unsigned short recursion_depth;
    enum Role role;
    struct _Function* param_of;
} Symbol;

Symbol* symbol_cursor;
Symbol* start_symbol;
Symbol* end_symbol;
unsigned long long symbol_id_counter;

Symbol* complex_mode;
unsigned long long symbol_counter;

Symbol* addSymbol(char *name, enum Type type, union Value value, enum ValueType value_type);
Symbol* updateSymbol(char *name, enum Type type, union Value value, enum ValueType value_type);
void removeSymbolByName(char *name);
void removeSymbol(Symbol* symbol);
void freeSymbol(Symbol* symbol);
Symbol* getSymbol(char *name);
Symbol* deepCopySymbol(Symbol* symbol, enum Type type, char *key);
Symbol* deepCopyComplex(char *name, Symbol* symbol);
char* getSymbolValueString(char *name);
long double getSymbolValueFloat(char *name);
bool getSymbolValueBool(char *name);
long long getSymbolValueInt(char *name);
char* getSymbolValueString_NullIfNotString(Symbol* symbol);
long long getSymbolValueInt_ZeroIfNotInt(Symbol* symbol);
void printSymbolValue(Symbol* symbol, bool is_complex);
void printSymbolValueEndWith(Symbol* symbol, char *end);
void printSymbolValueEndWithNewLine(Symbol* symbol);
bool isDefined(char *name);
void addSymbolToComplex(Symbol* symbol);
void printSymbolTable();
Symbol* addSymbolBool(char *name, bool b);
void updateSymbolBool(char *name, bool b);
Symbol* addSymbolInt(char *name, long long i);
void updateSymbolInt(char *name, long long i);
Symbol* addSymbolFloat(char *name, long double f);
void updateSymbolFloat(char *name, long double f);
Symbol* addSymbolString(char *name, char *s);
void updateSymbolString(char *name, char *s);
void addSymbolArray(char *name);
Symbol* createCloneFromSymbolByName(char *clone_name, enum Type type, char *name, enum Type extra_type);
Symbol* createCloneFromComplexElement(char *clone_name, enum Type type, char *name, unsigned long long symbol_id, enum Type extra_type);
Symbol* createCloneFromSymbol(char *clone_name, enum Type type, Symbol* symbol, enum Type extra_type);
Symbol* updateSymbolByClonning(char *clone_name, Symbol* symbol);
Symbol* updateSymbolByClonningName(char *clone_name, char *name);
Symbol* updateSymbolByClonningComplexElement(char *clone_name, char *name, unsigned long long symbol_id);
bool isComplexIllegal(enum Type type);
void finishComplexMode(char *name, enum Type type);
Symbol* getArrayElement(char *name, long long i);
void cloneSymbolToComplex(char *name, char *key);
Symbol* getComplexElement(char *name, long long i, char *key);
Symbol* getComplexElementBySymbolId(char *name, unsigned long long symbol_id);
void updateComplexElement(char *name, unsigned long long symbol_id, enum Type type, union Value value);
void updateComplexElementBool(char* name, unsigned long long symbol_id, bool b);
void updateComplexElementInt(char* name, unsigned long long symbol_id, long long i);
void updateComplexElementFloat(char* name, unsigned long long symbol_id, long double f);
void updateComplexElementString(char* name, unsigned long long symbol_id, char *s);
void updateComplexElementSymbol(char* name, unsigned long long symbol_id, char* source_name);
void removeComplexElement(char *name, unsigned long long symbol_id);
void addSymbolDict(char *name);
void addSymbolAnyString(char *name, char *s);
void addSymbolAnyInt(char *name, long long i);
void addSymbolAnyFloat(char *name, long double f);
void addSymbolAnyBool(char *name, bool b);
Symbol* getDictElement(char *name, char *key);
_Function* getCurrentScope();
Symbol* getSymbolFunctionParameter(char *name);
void freeAllSymbols();
Symbol* assignByTypeCasting(Symbol* clone_symbol, Symbol* symbol);
Symbol* createSymbolWithoutValueType(char *name, enum Type type);
void removeSymbolsByScope(_Function* scope);
long long incrementThenAssign(char *name, long long i);
long long assignThenIncrement(char *name, long long i);
char* getTypeName(unsigned i);
char* getValueTypeName(unsigned i);

#endif
