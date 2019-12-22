#include <stdio.h>
#include <string.h>
#include "errors.h"

enum Type { BOOL, INT, CHAR, STRING, FLOAT };

typedef struct {
    char *name;
    enum Type type;
    union Value {
        bool b;
        int i;
        char c;
        char *s;
        float f;
    } value;
} Symbol;

Symbol symbol_table[100];
int symbol_counter=0;

void addSymbol(char *name, enum Type type, bool b) {
    Symbol variable = {name, type, b};
    symbol_table[symbol_counter] = variable;
    symbol_counter++;
}

Symbol* getSymbol(char *name) {
    for ( int i = 0; i < sizeof(symbol_table) / sizeof(Symbol); i++) {
        if (strcmp(&symbol_table[i].name, &name)) {
            return &symbol_table[i];
        }
    }
}

void printSymbolValue(Symbol* symbol) {
    char subject[2] = "\0";
    switch (symbol->type)
    {
        case BOOL:
            printf("%i", symbol->value);
            break;
        case INT:
            printf("%i", symbol->value);
            break;
        case FLOAT:
            printf("%f", symbol->value);
            break;
        case CHAR:
            printf("%s", symbol->value);
            break;
        case STRING:
            printf("%s", symbol->value);
            break;
        default:
            subject[0] = symbol->type;
            throw_error(1, subject);
            break;
    }
}

int isDefined(char *name) {
    for ( int i = 0; i < sizeof(symbol_table) / sizeof(Symbol); i++) {
        if (symbol_table[i].name == name) {
            return 1;
        }
    }
    return 0;
}
