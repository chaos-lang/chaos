#include "symbol.h"

symbol_counter = 0;

Symbol* addSymbol(char *name, enum Type type, union Value value) {
    symbol_cursor = start_symbol;

    Symbol* symbol;
    symbol = (struct Symbol*)malloc(sizeof(Symbol));

    if (complex_mode != NULL && complex_mode->type == DICT) {
        symbol->key = name;
    } else {
        if (isDefined(name)) throw_error(2, name);
        symbol->name = name;
    }

    symbol->type = type;
    symbol->value.b = value.b;
    symbol->value.i = value.i;
    symbol->value.c = value.c;
    symbol->value.s = value.s;
    symbol->value.f = value.f;
    symbol->children_count = 0;
    symbol->scope = getCurrentScope();

    if (start_symbol == NULL) {
        start_symbol = symbol;
        end_symbol = symbol;
    } else {
        end_symbol->next = symbol;
        symbol->previous = end_symbol;
        end_symbol = symbol;
        end_symbol->next = NULL;
    }

    addSymbolToComplex(symbol);

    return symbol;
}

Symbol* updateSymbol(char *name, enum Type type, union Value value) {
    Symbol* symbol = getSymbol(name);

    if (symbol->type != ANY && symbol->type != type) throw_error(8, name);

    symbol->secondary_type = type;
    symbol->value.b = value.b;
    symbol->value.i = value.i;
    symbol->value.c = value.c;
    symbol->value.s = value.s;
    symbol->value.f = value.f;

    return symbol;
}

void removeSymbolByName(char *name) {
    Symbol* symbol = getSymbol(name);

    if (symbol->type == ARRAY || symbol->type == DICT) {
        for (int i = 0; i < symbol->children_count; i++) {
            removeSymbol(symbol->children[i]);
        }
    }
    removeSymbol(symbol);
}

void removeSymbol(Symbol* symbol) {
    Symbol* previous_symbol = symbol->previous;
    Symbol* next_symbol = symbol->next;

    if (previous_symbol == NULL && next_symbol == NULL) {
        start_symbol = NULL;
        end_symbol = NULL;
        free(symbol);
        return;
    } else if (previous_symbol == NULL) {
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
        if (
            symbol_cursor->name != NULL &&
            strcmp(symbol_cursor->name, name) == 0 &&
            symbol_cursor->scope == getCurrentScope()
        ) {
            Symbol* symbol = symbol_cursor;
            return symbol;
        }
        symbol_cursor = symbol_cursor->next;
    }
    throw_error(3, name);
}

Symbol* deepCopySymbol(Symbol* symbol, char *key) {
    return addSymbol(key, symbol->type, symbol->value);
}

Symbol* deepCopySymbolAny(Symbol* symbol, char *key) {
    Symbol* clone_symbol = addSymbol(key, ANY, symbol->value);
    clone_symbol->secondary_type = symbol->type;
    return clone_symbol;
}

Symbol* deepCopyComplex(char *name, Symbol* symbol) {
    if (symbol->type == ARRAY) {
        addSymbolArray(NULL);
    } else if (symbol->type == DICT) {
        addSymbolDict(NULL);
    } else {
        throw_error(12, symbol->name);
    }

    for (int i = 0; i < symbol->children_count; i++) {
        Symbol* child = symbol->children[i];
        deepCopySymbol(child, child->key);
    }

    Symbol* symbol_return = complex_mode;
    finishComplexMode(name, ANY);

    return symbol_return;
}

float getSymbolValueFloat(char *name) {
    Symbol* symbol = getSymbol(name);
    char type[2] = "\0";
    float value;
    switch (symbol->type)
    {
        case BOOL:
            value = symbol->value.b ? 1.0 : 0.0;
            return value;
            break;
        case INT:
            value = (float)symbol->value.i;
            return value;
            break;
        case FLOAT:
            value = symbol->value.f;
            return value;
            break;
        default:
            type[0] = symbol->type;
            throw_error(1, type);
            break;
    }
}

void printSymbolValue(Symbol* symbol, bool is_complex) {
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
            printf("%g", symbol->value.f);
            break;
        case CHAR:
            printf("%c", symbol->value.c);
            break;
        case STRING:
            if (is_complex) {
                printf("'%s'", symbol->value.s);
            } else {
                printf("%s", symbol->value.s);
            }
            break;
        case ARRAY:
            printf("[");
            for (int i = 0; i < symbol->children_count; i++) {
                printSymbolValue(symbol->children[i], true);
                if (i + 1 != symbol->children_count) {
                    printf(", ");
                }
            }
            printf("]");
            break;
        case DICT:
            printf("{");
            for (int i = 0; i < symbol->children_count; i++) {
                Symbol* child = symbol->children[i];
                printf("'%s': ", child->key);
                printSymbolValue(child, true);
                if (i + 1 != symbol->children_count) {
                    printf(", ");
                }
            }
            printf("}");
            break;
        case ANY:
            switch (symbol->secondary_type)
            {
            case STRING:
                printf("%s",symbol->value.s);
                break;
            case INT:
                printf("%i",symbol->value.i);
                break;
            case FLOAT:
                printf("%g",symbol->value.f);
                break;
            case BOOL:
                printf("%s", symbol->value.b ? "true" : "false");
                break;
            }
            break;
        default:
            type[0] = symbol->type;
            throw_error(1, type);
            break;
    }
}

void printSymbolValueEndWith(Symbol* symbol, char *end)
{
    printSymbolValue(symbol, false);
    printf("%s", end);
}

void printSymbolValueEndWithNewLine(Symbol* symbol)
{
    printSymbolValueEndWith(symbol, "\n");
}

bool isDefined(char *name) {
    symbol_cursor = start_symbol;
    while (symbol_cursor != NULL) {
        if (
            symbol_cursor->name != NULL &&
            name != NULL && strcmp(symbol_cursor->name, name) == 0 &&
            symbol_cursor->scope == getCurrentScope()
        ) {
            return true;
        }
        symbol_cursor = symbol_cursor->next;
    }
    return false;
}

void addSymbolToComplex(Symbol* symbol) {
    if (complex_mode == NULL) return;

    complex_mode->children = realloc(
        complex_mode->children,
        sizeof(Symbol) * ++symbol_counter
    );

    if (complex_mode->children == NULL) {
        throw_error(4, complex_mode->name);
    }

    complex_mode->children[symbol_counter - 1] = symbol;
}

void printSymbolTable() {
    Symbol *ptr1 = start_symbol;
    printf("[start] =>\n");
    while(ptr1 != NULL) {
        Function* scope1 = ptr1->scope;
        printf("\t{name: %s, scope: %s, type: %i, 2nd_type: %i} =>\n", ptr1->name, scope1->name, ptr1->type, ptr1->secondary_type);
        ptr1 = ptr1->next;
    }
    printf("[end]\n");
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
    complex_mode = addSymbol(name, ARRAY, value);
}

Symbol* createCloneFromSymbolByName(char *clone_name, enum Type type, char *name, enum Type extra_type) {
    Symbol* symbol = getSymbol(name);

    return createCloneFromSymbol(clone_name, type, symbol, extra_type);
}

Symbol* createCloneFromSymbol(char *clone_name, enum Type type, Symbol* symbol, enum Type extra_type) {
    if (type != ANY) {
        if (type == NUMBER) {
            if (symbol->type != INT && symbol->type != FLOAT) {
                throw_error(8, clone_name);
            }
        } else if (symbol->type != type) {
            throw_error(8, clone_name);
        }
    }

    Symbol* clone_symbol;
    if (symbol->type == ARRAY || symbol->type == DICT) {
        if (symbol->secondary_type != extra_type) throw_error(8, clone_name);
        clone_symbol = deepCopyComplex(clone_name, symbol);
    } else {
        if (type == ANY) {
            clone_symbol = deepCopySymbolAny(symbol, NULL);
        } else {
            clone_symbol = deepCopySymbol(symbol, NULL);
        }
        clone_symbol->name = clone_name;
    }
    return clone_symbol;
}

Symbol* updateSymbolByClonning(char *clone_name, char *name) {
    Symbol* symbol = getSymbol(name);
    Symbol* clone_symbol = getSymbol(clone_name);

    if (clone_symbol->type != ANY) {
        if (symbol->type == NUMBER) {
            if (clone_symbol->type != INT && clone_symbol->type != FLOAT) {
                throw_error(8, clone_name);
            }
        } else {
            if (clone_symbol->type != symbol->type) {
                throw_error(8, clone_name);
            }
        }
    }

    if (clone_symbol->type == ARRAY) throw_error(9, clone_name);

    Symbol* temp_symbol = clone_symbol;

    if (clone_symbol->type == ANY) {
        clone_symbol = deepCopySymbolAny(symbol, NULL);
    } else {
        clone_symbol = deepCopySymbol(symbol, NULL);
    }
    clone_symbol->name = clone_name;

    removeSymbol(temp_symbol);
    return clone_symbol;
}

bool isComplexIllegal(enum Type type) {
    if (complex_mode != NULL && type != ANY) {
        for (int i = 0; i < complex_mode->children_count; i++) {
            Symbol* symbol = complex_mode->children[i];
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

void finishComplexMode(char *name, enum Type type) {
    complex_mode->children_count = symbol_counter;
    complex_mode->name = name;
    complex_mode->secondary_type = type;
    if (isComplexIllegal(type)) throw_error(5, complex_mode->name);
    complex_mode = NULL;
    symbol_counter = 0;
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

void cloneSymbolToComplex(char *name, char *key) {
    deepCopySymbol(getSymbol(name), key);
}

void updateComplexElement(char *name, int i, char *key, enum Type type, union Value value) {
    Symbol* complex = getSymbol(name);
    if (complex->secondary_type != ANY) {
        if (complex->secondary_type == NUMBER) {
            if (type != INT && type != FLOAT) {
                throw_error(5, complex->name);
            }
        } else {
            if (complex->secondary_type != type) {
                throw_error(5, complex->name);
            }
        }
    }

    Symbol* symbol;
    if (complex->type == ARRAY) {
        symbol = getArrayElement(name, i);
    } else if (complex->type == DICT) {
        symbol = getDictElement(name, key);
    } else {
        throw_error(12, complex->name);
    }
    symbol->type = type;
    symbol->value.b = value.b;
    symbol->value.i = value.i;
    symbol->value.c = value.c;
    symbol->value.s = value.s;
    symbol->value.f = value.f;
}

void updateComplexElementBool(char* name, int index, char *key, bool b) {
    union Value value;
    value.b = b;
    updateComplexElement(name, index, key, BOOL, value);
}

void updateComplexElementInt(char* name, int index, char *key, int i) {
    union Value value;
    value.i = i;
    updateComplexElement(name, index, key, INT, value);
}

void updateComplexElementFloat(char* name, int index, char *key, float f) {
    union Value value;
    value.f = f;
    updateComplexElement(name, index, key, FLOAT, value);
}

void updateComplexElementString(char* name, int index, char *key, char *s) {
    union Value value;
    value.s = s;
    updateComplexElement(name, index, key, STRING, value);
}

void updateComplexElementSymbol(char* name, int index, char *key, char* source_name) {
    Symbol* complex = getSymbol(name);
    Symbol* source = getSymbol(source_name);
    if (complex->secondary_type != ANY) {
        if (complex->secondary_type == NUMBER) {
            if (source->type != INT && source->type != FLOAT) {
                throw_error(5, complex->name);
            }
        } else {
            if (complex->secondary_type != source->type) {
                throw_error(5, complex->name);
            }
        }
    }

    Symbol* symbol;
    if (complex->type == ARRAY) {
        symbol = getArrayElement(name, index);
        removeSymbol(symbol);
        complex->children[index] = deepCopySymbol(source, NULL);
    } else if (complex->type == DICT) {
        removeComplexElement(name, NULL, key);
        complex_mode = complex;
        symbol_counter = complex->children_count;
        deepCopySymbol(source, key);
        finishComplexMode(complex->name, complex->secondary_type);
    } else {
        throw_error(12, complex->name);
    }
}

void removeComplexElement(char *name, int i, char *key) {
    Symbol* complex = getSymbol(name);
    Symbol* symbol;
    if (complex->type == ARRAY) {
        symbol = getArrayElement(name, i);
    } else if (complex->type == DICT) {
        symbol = getDictElement(name, key);
        for (int j = 0; j < complex->children_count; j++) {
            Symbol* child = complex->children[j];
            if (strcmp(child->key, key) == 0) {
                i = j;
            }
        }
    } else {
        throw_error(12, name);
    }
    Symbol** temp = malloc((complex->children_count - 1) * sizeof(Symbol));

    // Copy everything before the index
    if (i != 0)
        memcpy(temp, complex->children, i * sizeof(Symbol));

    // Copy everything after the index
    if (i != complex->children_count - 1)
        memcpy(temp + i, complex->children + i + 1, (complex->children_count - i - 1) * sizeof(Symbol));

    free(complex->children);
    complex->children = temp;
    removeSymbol(symbol);
    complex->children_count--;
}

void addSymbolDict(char *name) {
    union Value value;
    complex_mode = addSymbol(name, DICT, value);
}

Symbol* getDictElement(char *name, char *key) {
    Symbol* symbol = getSymbol(name);
    if (symbol->type != DICT) throw_error(10, name);

    for (int i = 0; i < symbol->children_count; i++) {
        Symbol* child = symbol->children[i];
        if (child->key != NULL && strcmp(child->key, key) == 0) {
            return child;
        }
    }
    throw_error(11, key);
}

void addSymbolAnyString(char *name, char *s) {
    union Value value;
    value.s = s;
    Symbol* symbol = addSymbol(name, ANY, value);
    symbol->secondary_type = STRING;
}

void addSymbolAnyInt(char *name, int i) {
    union Value value;
    value.i = i;
    Symbol* symbol = addSymbol(name, ANY, value);
    symbol->secondary_type = INT;
}

void addSymbolAnyFloat(char *name, float f) {
    union Value value;
    value.f = f;
    Symbol* symbol = addSymbol(name, ANY, value);
    symbol->secondary_type = FLOAT;
}

void addSymbolAnyBool(char *name, bool b) {
    union Value value;
    value.b = b;
    Symbol* symbol = addSymbol(name, ANY, value);
    symbol->secondary_type = BOOL;
}

Function* getCurrentScope() {
    if (scope_override != NULL) return scope_override;

    if (executed_function != NULL) {
        return executed_function;
    } else if (function_parameters_mode != NULL) {
        return function_parameters_mode;
    } else {
        return main_function;
    }
}

Symbol* getSymbolFunctionParameter(char *name) {
    if (executed_function != NULL) {
        scope_override = executed_function;
    } else {
        scope_override = main_function;
    }

    Symbol* symbol = getSymbol(name);
    scope_override = NULL;
    return symbol;
}
