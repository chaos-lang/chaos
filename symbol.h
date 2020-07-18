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

typedef struct symbol_array {
    Symbol** arr;
    unsigned long* child_counter;
    unsigned capacity, size;
} symbol_array;

symbol_array complex_mode_stack;
symbol_array nested_complex_mode_stack;
bool disable_complex_mode;

typedef struct symbol_id_array {
    unsigned long long* arr;
    unsigned capacity, size;
} symbol_id_array;

symbol_id_array left_right_bracket_stack;
Symbol* variable_complex_element;
unsigned long long variable_complex_element_symbol_id;

Symbol* addSymbol(char *name, enum Type type, union Value value, enum ValueType value_type);
Symbol* updateSymbol(char *name, enum Type type, union Value value, enum ValueType value_type);
void removeSymbolByName(char *name);
void removeSymbol(Symbol* symbol);
void freeSymbol(Symbol* symbol);
Symbol* getSymbol(char *name);
Symbol* getSymbolById(unsigned long long id);
Symbol* deepCopySymbol(Symbol* symbol, enum Type type, char *key);
Symbol* deepCopyComplex(char *name, Symbol* symbol);
char* getSymbolValueString(char *name);
char* _getSymbolValueString(Symbol* symbol);
long double getSymbolValueFloat(char *name);
long double _getSymbolValueFloat(Symbol* symbol);
bool getSymbolValueBool(char *name);
bool _getSymbolValueBool(Symbol* symbol);
long long getSymbolValueInt(char *name);
long long _getSymbolValueInt(Symbol* symbol);
char* getSymbolValueString_NullIfNotString(Symbol* symbol);
long long getSymbolValueInt_ZeroIfNotInt(Symbol* symbol);
void printSymbolValue(Symbol* symbol, bool is_complex, bool pretty, bool escaped, unsigned long iter);
void printSymbolValueEndWith(Symbol* symbol, char *end, bool pretty, bool escaped);
void printSymbolValueEndWithNewLine(Symbol* symbol, bool pretty, bool escaped);
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
void addSymbolList(char *name);
Symbol* createCloneFromSymbolByName(char *clone_name, enum Type type, char *name, enum Type extra_type);
Symbol* createCloneFromComplexElement(char *clone_name, enum Type type, char *name, enum Type extra_type);
Symbol* createCloneFromSymbol(char *clone_name, enum Type type, Symbol* symbol, enum Type extra_type);
Symbol* updateSymbolByClonning(char *clone_name, Symbol* symbol);
Symbol* updateSymbolByClonningName(char *clone_name, char *name);
Symbol* updateSymbolByClonningComplexElement(char *clone_name, char *name);
enum Type isComplexIllegal(enum Type type);
Symbol* finishComplexMode(char *name, enum Type type);
void finishComplexModeWithUpdate(char *name);
void _finishComplexModeWithUpdate(Symbol* symbol);
Symbol* getListElement(Symbol* symbol, long long i);
void cloneSymbolToComplex(char *name, char *key);
Symbol* getComplexElement(Symbol* complex, long long i, char *key);
Symbol* getComplexElementBySymbolId(Symbol* complex, unsigned long long symbol_id);
void updateComplexElementComplex();
void updateComplexElementWrapper(enum Type type, union Value value, enum ValueType value_type);
void updateComplexElement(Symbol* complex, unsigned long long symbol_id, enum Type type, union Value value, enum ValueType value_type);
void updateComplexElementBool(bool b);
void updateComplexElementInt(long long i);
void updateComplexElementFloat(long double f);
void updateComplexElementString(char *s);
void updateComplexElementSymbol(Symbol* source);
void _updateComplexElementSymbol(Symbol* complex, unsigned long long symbol_id, Symbol* source);
void removeComplexElementByLeftRightBracketStack(char *name);
void removeComplexElement(Symbol* complex, unsigned long long symbol_id);
void addSymbolDict(char *name);
void addSymbolAnyString(char *name, char *s);
void addSymbolAnyInt(char *name, long long i);
void addSymbolAnyFloat(char *name, long double f);
void addSymbolAnyBool(char *name, bool b);
Symbol* getDictElement(Symbol* symbol, char *key);
_Function* getCurrentScope();
Symbol* getSymbolFunctionParameter(char *name);
void freeAllSymbols();
Symbol* assignByTypeCasting(Symbol* clone_symbol, Symbol* symbol);
bool symbolValueByTypeCastingToBool(Symbol* symbol);
long long symbolValueByTypeCastingToInt(Symbol* symbol);
long double symbolValueByTypeCastingToFloat(Symbol* symbol);
char* symbolValueByTypeCastingToString(Symbol* symbol);
Symbol* createSymbolWithoutValueType(char *name, enum Type type);
void removeSymbolsByScope(_Function* scope);
long long incrementThenAssign(char *name, long long i);
long long assignThenIncrement(char *name, long long i);
char* getTypeName(unsigned i);
char* getValueTypeName(unsigned i);
void pushComplexModeStack(Symbol* complex_mode);
void popComplexModeStack();
void freeComplexModeStack();
bool isComplexMode();
bool isNestedComplexMode();
Symbol* getComplexMode();
void pushLeftRightBracketStack(unsigned long long symbol_id);
unsigned long long popLeftRightBracketStack();
void freeLeftRightBracketStack();
Symbol* getComplexElementThroughLeftRightBracketStack(char *name, unsigned long inverse_nested);
void removeChildrenOfComplex(Symbol* symbol);
bool isComplex(Symbol* symbol);
void buildVariableComplexElement(char *name, char *key);
void pushNestedComplexModeStack(Symbol* complex_mode);
void popNestedComplexModeStack(char *key);
void freeNestedComplexModeStack();

#endif
