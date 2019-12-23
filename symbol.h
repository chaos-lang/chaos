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
    struct Symbol* previous;
    struct Symbol* next;
} Symbol;

Symbol* symbol_cursor;
Symbol* start_symbol;
Symbol* end_symbol;

bool isDefined(char *name);

void addSymbol(char *name, enum Type type, union Value value) {
    if (isDefined(name)) throw_error(2, name);
    symbol_cursor = start_symbol;

    Symbol* symbol;
    symbol = (struct Symbol*)malloc(sizeof(Symbol));
    symbol->name = name;
    symbol->type = type;
    symbol->value.b = value.b;
    symbol->value.i = value.i;
    symbol->value.c = value.c;
    symbol->value.s = value.s;
    symbol->value.f = value.f;
    if (start_symbol == NULL) {
        start_symbol = symbol;
        end_symbol = symbol;
    } else {
        end_symbol->next = symbol;
        symbol->previous = end_symbol;
        end_symbol = symbol;
        end_symbol->next = NULL;
    }
}

int updateSymbol(char *name, union Value value) {
    symbol_cursor = start_symbol;
    while (symbol_cursor != NULL) {
        if (strcmp(symbol_cursor->name, name) == 0) {
            symbol_cursor->value.b = value.b;
            symbol_cursor->value.i = value.i;
            symbol_cursor->value.c = value.c;
            symbol_cursor->value.s = value.s;
            symbol_cursor->value.f = value.f;
            return 0;
        }
        symbol_cursor = symbol_cursor->next;
    }
    throw_error(3, name);
}

int removeSymbol(char *name) {
    symbol_cursor = start_symbol;
    while (symbol_cursor != NULL) {
        if (strcmp(symbol_cursor->name, name) == 0) {
            Symbol* previous_symbol = symbol_cursor->previous;
            Symbol* next_symbol = symbol_cursor->next;

            if (previous_symbol == NULL) {
                start_symbol = next_symbol;
                start_symbol->previous = NULL;
            } else if (next_symbol == NULL) {
                end_symbol = previous_symbol;
                end_symbol->next = NULL;
            } else {
                previous_symbol->next = next_symbol;
                next_symbol->previous = previous_symbol;
            }

            free(symbol_cursor);
            return 0;
        }
        symbol_cursor = symbol_cursor->next;
    }
    throw_error(3, name);
}

Symbol* getSymbol(char *name) {
    symbol_cursor = start_symbol;
    while (symbol_cursor != NULL) {
        if (strcmp(symbol_cursor->name, name) == 0) {
            Symbol* symbol = symbol_cursor;
            return symbol;
        }
        symbol_cursor = symbol_cursor->next;
    }
    throw_error(3, name);
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
    printf("\n");
}

bool isDefined(char *name) {
    symbol_cursor = start_symbol;
    while (symbol_cursor != NULL) {
        if (strcmp(symbol_cursor->name, name) == 0) {
            return true;
        }
        symbol_cursor = symbol_cursor->next;
    }
    return false;
}

void printSymbolTable() {
    //start from the beginning
    Symbol *ptr1 = start_symbol;
    printf("[start] =>");
    while(ptr1 != NULL) {
        printf(" %s =>", ptr1->name);
        ptr1 = ptr1->next;
    }
    printf(" [end]\n");

    //start from the end
    Symbol *ptr2 = end_symbol;
    printf("[end] =>");
    while(ptr2 != NULL) {
        printf(" %s =>", ptr2->name);
        ptr2 = ptr2->previous;
    }
    printf(" [start]\n");
}

void addSymbolBool(char *name, enum Type type, bool b) {
    union Value value;
    value.b = b;
    addSymbol(name, type, value);
}

void updateSymbolBool(char *name, bool b) {
    union Value value;
    value.b = b;
    updateSymbol(name, value);
}

void addSymbolInt(char *name, enum Type type, int i) {
    union Value value;
    value.i = i;
    addSymbol(name, type, value);
}

void updateSymbolInt(char *name, int i) {
    union Value value;
    value.i = i;
    updateSymbol(name, value);
}

void addSymbolFloat(char *name, enum Type type, float f) {
    union Value value;
    value.f = f;
    addSymbol(name, type, value);
}

void updateSymbolFloat(char *name, float f) {
    union Value value;
    value.f = f;
    updateSymbol(name, value);
}

void addSymbolString(char *name, enum Type type, char *s) {
    union Value value;
    value.s = s;
    addSymbol(name, type, value);
}

void updateSymbolString(char *name, char *s) {
    union Value value;
    value.s = s;
    updateSymbol(name, value);
}
