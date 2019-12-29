#include <stdio.h>
#include <string.h>
#include "errors.h"

enum Type { BOOL, INT, CHAR, STRING, FLOAT, NUMBER, ANY, ARRAY };

typedef struct {
    char *name;
    enum Type type;
    enum Type array_type;
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
} Symbol;

Symbol* symbol_cursor;
Symbol* start_symbol;
Symbol* end_symbol;

Symbol* array_mode = NULL;
int array_symbol_counter = 0;

Symbol* getSymbol(char *name);
bool isDefined(char *name);
void removeSymbol(Symbol* symbol);
void addSymbolToArray(Symbol* symbol);

Symbol* addSymbol(char *name, enum Type type, union Value value) {
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
    symbol->children_count = 0;

    if (start_symbol == NULL) {
        start_symbol = symbol;
        end_symbol = symbol;
    } else {
        end_symbol->next = symbol;
        symbol->previous = end_symbol;
        end_symbol = symbol;
        end_symbol->next = NULL;
    }

    addSymbolToArray(symbol);

    return symbol;
}

Symbol* updateSymbol(char *name, enum Type type, union Value value) {
    Symbol* symbol = getSymbol(name);
    if (symbol->type != type) throw_error(8, name);

    symbol->value.b = value.b;
    symbol->value.i = value.i;
    symbol->value.c = value.c;
    symbol->value.s = value.s;
    symbol->value.f = value.f;
    return symbol;
}

int removeSymbolByName(char *name) {
    symbol_cursor = start_symbol;
    while (symbol_cursor != NULL) {
        if (symbol_cursor->name != NULL && strcmp(symbol_cursor->name, name) == 0) {
            if (symbol_cursor->type == ARRAY) {
                for (int i = 0; i < symbol_cursor->children_count; i++) {
                    removeSymbol(symbol_cursor->children[i]);
                }
            }
            removeSymbol(symbol_cursor);
            return 0;
        }
        symbol_cursor = symbol_cursor->next;
    }
    throw_error(3, name);
}

void removeSymbol(Symbol* symbol) {
    Symbol* previous_symbol = symbol->previous;
    Symbol* next_symbol = symbol->next;

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

    free(symbol);
}

Symbol* getSymbol(char *name) {
    symbol_cursor = start_symbol;
    while (symbol_cursor != NULL) {
        if (symbol_cursor->name != NULL && strcmp(symbol_cursor->name, name) == 0) {
            Symbol* symbol = symbol_cursor;
            return symbol;
        }
        symbol_cursor = symbol_cursor->next;
    }
    throw_error(3, name);
}

Symbol* deepCopySymbol(Symbol* symbol) {
    return addSymbol(NULL, symbol->type, symbol->value);
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
        case ARRAY:
            printf("[");
            for (int i = 0; i < symbol->children_count; i++) {
                printSymbolValue(symbol->children[i]);
                if (i + 1 != symbol->children_count) {
                    printf(", ");
                }
            }
            printf("]");
            break;
        default:
            type[0] = symbol->type;
            throw_error(1, type);
            break;
    }
}

void printSymbolValueEndWith(Symbol* symbol, char *end)
{
    printSymbolValue(symbol);
    printf("%s", end);
}

void printSymbolValueEndWithNewLine(Symbol* symbol)
{
    printSymbolValueEndWith(symbol, "\n");
}

bool isDefined(char *name) {
    symbol_cursor = start_symbol;
    while (symbol_cursor != NULL) {
        if (symbol_cursor->name != NULL && name != NULL && strcmp(symbol_cursor->name, name) == 0) {
            return true;
        }
        symbol_cursor = symbol_cursor->next;
    }
    return false;
}

void addSymbolToArray(Symbol* symbol) {
    if (array_mode != NULL) {
        array_mode->children = realloc(
            array_mode->children,
            sizeof(Symbol) * ++array_symbol_counter
        );

        if (array_mode->children == NULL) {
            throw_error(4, array_mode->name);
        }

        array_mode->children[array_symbol_counter - 1] = symbol;
    }
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

void addSymbolBool(char *name, bool b) {
    union Value value;
    value.b = b;
    addSymbol(name, BOOL, value);
}

void updateSymbolBool(char *name, bool b) {
    union Value value;
    value.b = b;
    updateSymbol(name, BOOL, value);
}

void addSymbolInt(char *name, int i) {
    union Value value;
    value.i = i;
    addSymbol(name, INT, value);
}

void updateSymbolInt(char *name, int i) {
    union Value value;
    value.i = i;
    updateSymbol(name, INT, value);
}

void addSymbolFloat(char *name, float f) {
    union Value value;
    value.f = f;
    addSymbol(name, FLOAT, value);
}

void updateSymbolFloat(char *name, float f) {
    union Value value;
    value.f = f;
    updateSymbol(name, FLOAT, value);
}

void addSymbolString(char *name, char *s) {
    union Value value;
    value.s = s;
    addSymbol(name, STRING, value);
}

void updateSymbolString(char *name, char *s) {
    union Value value;
    value.s = s;
    updateSymbol(name, STRING, value);
}

void addSymbolArray(char *name) {
    union Value value;
    array_mode = addSymbol(name, ARRAY, value);
}

Symbol* createCloneFromSymbol(char *clone_name, enum Type type, char *name) {
    Symbol* symbol = getSymbol(name);

    if (type == NUMBER) {
        if (symbol->type != INT && symbol->type != FLOAT) {
            throw_error(8, clone_name);
        }
    } else {
        if (symbol->type != type) {
            throw_error(8, clone_name);
        }
    }

    Symbol* clone_symbol = deepCopySymbol(symbol);
    clone_symbol->name = clone_name;
    return clone_symbol;
}

Symbol* updateSymbolByClonning(char *clone_name, char *name) {
    Symbol* symbol = getSymbol(name);
    Symbol* clone_symbol = getSymbol(clone_name);

    if (symbol->type == NUMBER) {
        if (clone_symbol->type != INT && clone_symbol->type != FLOAT) {
            throw_error(8, clone_name);
        }
    } else {
        if (clone_symbol->type != symbol->type) {
            throw_error(8, clone_name);
        }
    }

    Symbol* temp_symbol = clone_symbol;

    clone_symbol = deepCopySymbol(symbol);
    clone_symbol->name = clone_name;

    removeSymbol(temp_symbol);
    return clone_symbol;
}

bool isArrayIllegal(enum Type type) {
    if (array_mode != NULL && type != ANY) {
        for (int i = 0; i < array_mode->children_count; i++) {
            Symbol* symbol = array_mode->children[i];
            if (type == NUMBER) {
                if (symbol->type != INT && symbol->type != FLOAT) {
                    return true;
                }
            } else {
                if (symbol->type != type) {
                    return true;
                }
            }
        }
    }
    return false;
}

void finishArrayMode(char *name, enum Type type) {
    array_mode->children_count = array_symbol_counter;
    array_mode->name = name;
    array_mode->array_type = type;
    if (isArrayIllegal(type)) throw_error(5, array_mode->name);
    array_mode = NULL;
    array_symbol_counter = 0;
}

Symbol* getArrayElement(char *name, int i) {
    Symbol* symbol = getSymbol(name);
    if (symbol->type != ARRAY) throw_error(6, name);

    if (i < 0) {
        i = symbol->children_count + i;
    }

    if (i < 0 || i > symbol->children_count - 1) {
        char buffer[__ITOA_BUFFER_LENGTH__];
        throw_error(7, itoa(i, buffer, 10));
    }

    return symbol->children[i];
}

void cloneSymbolToArray(char *name) {
    deepCopySymbol(getSymbol(name));
}

void updateArrayElement(char *name, int i, enum Type type, union Value value) {
    Symbol* array = getSymbol(name);
    if (array->array_type != ANY) {
        if (array->array_type == NUMBER) {
            if (type != INT && type != FLOAT) {
                throw_error(5, array->name);
            }
        } else {
            if (array->array_type != type) {
                throw_error(5, array->name);
            }
        }
    }

    Symbol* symbol = getArrayElement(name, i);
    symbol->type = type;
    symbol->value.b = value.b;
    symbol->value.i = value.i;
    symbol->value.c = value.c;
    symbol->value.s = value.s;
    symbol->value.f = value.f;
}

void updateArrayElementBool(char* name, int index, bool b) {
    union Value value;
    value.b = b;
    updateArrayElement(name, index, BOOL, value);
}

void updateArrayElementInt(char* name, int index, int i) {
    union Value value;
    value.i = i;
    updateArrayElement(name, index, INT, value);
}

void updateArrayElementFloat(char* name, int index, float f) {
    union Value value;
    value.f = f;
    updateArrayElement(name, index, FLOAT, value);
}

void updateArrayElementString(char* name, int index, char *s) {
    union Value value;
    value.s = s;
    updateArrayElement(name, index, STRING, value);
}

void removeArrayElement(char *name, int i) {
    Symbol* array = getSymbol(name);
    Symbol* symbol = getArrayElement(name, i);
    Symbol** temp = malloc((array->children_count - 1) * sizeof(Symbol));

    // Copy everything before the index
    if (i != 0)
        memcpy(temp, array->children, i * sizeof(Symbol));

    // Copy everything after the index
    if (i != array->children_count - 1)
        memcpy(temp + i, array->children + i + 1, (array->children_count - i - 1) * sizeof(Symbol));

    free(array->children);
    array->children = temp;
    removeSymbol(symbol);
    array->children_count--;
}
