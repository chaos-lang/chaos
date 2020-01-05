#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "errors.h"
#include "utilities/helpers.h"

enum Type { BOOL, INT, CHAR, STRING, FLOAT, NUMBER, ANY, ARRAY, DICT };

typedef struct {
    char *name;
    enum Type type;
    enum Type secondary_type;
    union Value {
        bool b;
        int i;
        char c;
        char *s;
        float f;
    } value;
    struct Symbol* previous;
    struct Symbol* next;
    struct Symbol** children;
    int children_count;
    char *key;
} Symbol;

Symbol* symbol_cursor;
Symbol* start_symbol;
Symbol* end_symbol;

Symbol* complex_mode;
int symbol_counter;

Symbol* addSymbol(char *name, enum Type type, union Value value);
Symbol* updateSymbol(char *name, enum Type type, union Value value);
void removeSymbolByName(char *name);
void removeSymbol(Symbol* symbol);
Symbol* getSymbol(char *name);
Symbol* deepCopySymbol(Symbol* symbol, char *key);
void deepCopyComplex(char *name, Symbol* symbol);
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
Symbol* createCloneFromSymbol(char *clone_name, enum Type type, char *name, enum Type extra_type);
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
Symbol* getDictElement(char *name, char *key);

#endif
