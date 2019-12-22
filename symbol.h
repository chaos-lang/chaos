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

bool isDefined(char *name);

int addSymbol(char *name, enum Type type, bool b) {
    isDefined(name);

    Symbol variable = {name, type, b};
    symbol_table[symbol_counter] = variable;
    symbol_counter++;
}

Symbol* getSymbol(char *name) {
    for (int i = 0; i < sizeof(symbol_table) / sizeof(Symbol); i++) {
        if (strcmp(symbol_table[i].name, &name)) {
            return &symbol_table[i];
        }
    }
}

void printSymbolValue(Symbol* symbol) {
    char type[2] = "\0";
    switch (symbol->type)
    {
        case BOOL:
            printf("%s", symbol->value.b ? "true" : "false");
            break;
        case INT:
            printf("%i", symbol->value.i);
            break;
        case FLOAT:
            printf("%f", symbol->value.f);
            break;
        case CHAR:
            printf("%c", symbol->value.c);
            break;
        case STRING:
            printf("%s", symbol->value.s);
            break;
        default:
            type[0] = symbol->type;
            throw_error(1, type);
            break;
    }
}

bool isDefined(char *name) {
    for (int i = 0; i < sizeof(symbol_table) / sizeof(Symbol); i++) {
        if (symbol_table[i].name != NULL && strcmp(symbol_table[i].name, &name)) {
            throw_error(2, name);
            return true;
        }
    }
    return false;
}
