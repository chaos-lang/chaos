#include "symbol.h"

int symbol_counter = 0;
unsigned long long int symbol_id_counter = 0;

Symbol* addSymbol(char *name, enum Type type, union Value value, enum ValueType value_type) {
    symbol_cursor = start_symbol;

    Symbol* symbol;
    symbol = (struct Symbol*)calloc(1, sizeof(Symbol));
    symbol->id = symbol_id_counter++;

    if (complex_mode != NULL && complex_mode->type == DICT) {
        if (name != NULL) {
            symbol->key = malloc(1 + strlen(name));
            strcpy(symbol->key, name);
        }
    } else {
        if (isDefined(name)) {
            free(symbol);
            throw_error(E_VARIABLE_ALREADY_DEFINED, name);
        }
        if (name != NULL) {
            symbol->name = malloc(1 + strlen(name));
            strcpy(symbol->name, name);
        }
    }

    symbol->type = type;
    symbol->value = value;
    symbol->value_type = value_type;
    symbol->children_count = 0;
    symbol->scope = getCurrentScope();
    symbol->recursion_depth = recursion_depth;
    symbol->role = DEFAULT;

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
    #if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
    add_suggestion(name);
    #endif

    free(name);

    return symbol;
}

Symbol* updateSymbol(char *name, enum Type type, union Value value, enum ValueType value_type) {
    Symbol* symbol = getSymbol(name);

    if (symbol->type != ANY && symbol->type != type) throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, name);

    if (symbol->value_type == V_STRING) free(symbol->value.s);
    symbol->value = value;
    symbol->value_type = value_type;

    free(name);

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
        freeSymbol(symbol);
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

    freeSymbol(symbol);
}

void freeSymbol(Symbol* symbol) {
    if (symbol->value_type == V_STRING) free(symbol->value.s);
    if (symbol->children_count > 0) free(symbol->children);
    free(symbol->key);
    free(symbol->name);
    free(symbol->secondary_name);
    free(symbol);
}

Symbol* getSymbol(char *name) {
    symbol_cursor = start_symbol;
    while (symbol_cursor != NULL) {
        if (
            symbol_cursor->name != NULL &&
            strcmp(symbol_cursor->name, name) == 0 &&
            symbol_cursor->scope == getCurrentScope() &&
            symbol_cursor->recursion_depth == recursion_depth
        ) {
            Symbol* symbol = symbol_cursor;
            return symbol;
        }
        symbol_cursor = symbol_cursor->next;
    }
    throw_error(E_UNDEFINED_VARIABLE, name);
    return NULL;
}

Symbol* getSymbolById(unsigned long long int id) {
    symbol_cursor = start_symbol;
    while (symbol_cursor != NULL) {
        if (symbol_cursor->id == id) {
            Symbol* symbol = symbol_cursor;
            return symbol;
        }
        symbol_cursor = symbol_cursor->next;
    }
    throw_error(E_NO_VARIABLE_WITH_ID, "");
    return NULL;
}

Symbol* deepCopySymbol(Symbol* symbol, enum Type type, char *key) {
    Symbol* clone_symbol = addSymbol(key, type, symbol->value, symbol->value_type);
    if (symbol->value_type == V_STRING) {
        clone_symbol->value.s = malloc(1 + strlen(symbol->value.s));
        strcpy(clone_symbol->value.s, symbol->value.s);
    }
    return clone_symbol;
}

Symbol* deepCopyComplex(char *name, Symbol* symbol) {
    if (symbol->type == ARRAY) {
        addSymbolArray(NULL);
    } else if (symbol->type == DICT) {
        addSymbolDict(NULL);
    } else {
        free(name);
        throw_error(E_UNRECOGNIZED_COMPLEX_DATA_TYPE, symbol->name);
    }

    for (int i = 0; i < symbol->children_count; i++) {
        Symbol* child = symbol->children[i];
        char *key = NULL;
        if (child->key != NULL) {
            key = malloc(1 + strlen(child->key));
            strcpy(key, child->key);
        }
        deepCopySymbol(child, child->type, key);
    }

    Symbol* symbol_return = complex_mode;
    finishComplexMode(name, ANY);

    return symbol_return;
}

char* getSymbolValueString(char *name) {
    Symbol* symbol = getSymbol(name);
    free(name);
    char value_type[2] = "\0";
    char* value;
    if (symbol->value_type == V_STRING) {
        value = malloc(1 + strlen(symbol->value.s));
        strcpy(value, symbol->value.s);
        return value;
    } else {
        value_type[0] = symbol->value_type;
        throw_error(E_UNKNOWN_VARIABLE_TYPE, value_type);
    }
    return "";
}

float getSymbolValueFloat(char *name) {
    Symbol* symbol = getSymbol(name);
    free(name);
    char value_type[2] = "\0";
    float value;
    switch (symbol->value_type)
    {
        case V_BOOL:
            value = symbol->value.b ? 1.0 : 0.0;
            return value;
            break;
        case V_INT:
            value = (float)symbol->value.i;
            return value;
            break;
        case V_FLOAT:
            value = symbol->value.f;
            return value;
            break;
        default:
            value_type[0] = symbol->value_type;
            throw_error(E_UNKNOWN_VARIABLE_TYPE, value_type);
            break;
    }
    return 0.0;
}

bool getSymbolValueBool(char *name) {
    Symbol* symbol = getSymbol(name);
    free(name);
    bool value;
    switch (symbol->value_type)
    {
        case V_BOOL:
            value = symbol->value.b;
            return value;
            break;
        case V_INT:
            value = symbol->value.i > 0 ? true : false;
            return value;
            break;
        case V_FLOAT:
            value = symbol->value.f > 0.0 ? true : false;
            return value;
            break;
        default:
            value = symbol->value.s[0] != '\0' ? true : false;
            return value;
            break;
    }
    return false;
}

int getSymbolValueInt(char *name) {
    Symbol* symbol = getSymbol(name);
    free(name);
    char value_type[2] = "\0";
    int value;
    switch (symbol->value_type)
    {
        case V_BOOL:
            value = symbol->value.b ? 1 : 0;
            return value;
            break;
        case V_INT:
            value = symbol->value.i;
            return value;
            break;
        case V_FLOAT:
            value = (int)symbol->value.f;
            return value;
            break;
        default:
            value_type[0] = symbol->value_type;
            throw_error(E_UNKNOWN_VARIABLE_TYPE, value_type);
            break;
    }
    return 0;
}

char* getSymbolValueString_NullIfNotString(Symbol* symbol) {
    if (symbol->value_type != V_STRING) {
        return NULL;
    }

    char* value;
    value = malloc(1 + strlen(symbol->value.s));
    strcpy(value, symbol->value.s);
    return value;
}

int getSymbolValueInt_ZeroIfNotInt(Symbol* symbol) {
    if (symbol->value_type != V_INT && symbol->value_type != V_FLOAT) {
        return 0;
    }
    char value_type[2] = "\0";
    int value;
    switch (symbol->value_type)
    {
        case V_BOOL:
            value = symbol->value.b ? 1 : 0;
            break;
        case V_INT:
            value = symbol->value.i;
            break;
        case V_FLOAT:
            value = (int)symbol->value.f;
            break;
        default:
            value_type[0] = symbol->value_type;
            throw_error(E_UNKNOWN_VARIABLE_TYPE, value_type);
            break;
    }
    return value * symbol->sign;
}

void printSymbolValue(Symbol* symbol, bool is_complex) {
    char type[2] = "\0";

    switch (symbol->type)
    {
        case BOOL:
            printf("%s", symbol->value.b ? "true" : "false");
            break;
        case NUMBER:
            switch (symbol->value_type)
            {
                case V_INT:
                    printf("%i", symbol->value.i);
                    break;
                case V_FLOAT:
                    printf("%g", symbol->value.f);
                    break;
                default:
                    type[0] = symbol->value_type;
                    throw_error(E_UNEXPECTED_VALUE_TYPE, type);
                    break;
            }
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
            switch (symbol->value_type)
            {
                case V_STRING:
                    printf("%s", symbol->value.s);
                    break;
                case V_INT:
                    printf("%i", symbol->value.i);
                    break;
                case V_FLOAT:
                    printf("%g", symbol->value.f);
                    break;
                case V_BOOL:
                    printf("%s", symbol->value.b ? "true" : "false");
                    break;
                default:
                    type[0] = symbol->value_type;
                    throw_error(E_UNEXPECTED_VALUE_TYPE, type);
                    break;
            }
            break;
        default:
            type[0] = symbol->type;
            throw_error(E_UNKNOWN_VARIABLE_TYPE, type);
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
            symbol_cursor->scope == getCurrentScope() &&
            symbol_cursor->recursion_depth == recursion_depth
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
        throw_error(E_MEMORY_ALLOCATION_FOR_ARRAY_FAILED, complex_mode->name);
    }

    complex_mode->children[symbol_counter - 1] = symbol;
}

void printSymbolTable() {
    Symbol *symbol = start_symbol;
    printf("[start] =>\n");
    while(symbol != NULL) {
        _Function* scope = symbol->scope;
        printf(
            "\t{id: %llu, name: %s, 2nd_name: %s, key: %s, scope: %s, depth: %i, type: %i, 2nd_type: %i, value_type: %i, role: %i, param_of: %s} =>\n",
            symbol->id,
            symbol->name,
            symbol->secondary_name,
            symbol->key,
            scope->name,
            symbol->recursion_depth,
            symbol->type,
            symbol->secondary_type,
            symbol->value_type,
            symbol->role,
            symbol->param_of != NULL ? symbol->param_of->name : ""
        );
        symbol = symbol->next;
    }
    printf("[end]\n");
}

void addSymbolBool(char *name, bool b) {
    union Value value;
    value.b = b;
    addSymbol(name, BOOL, value, V_BOOL);
}

void updateSymbolBool(char *name, bool b) {
    union Value value;
    value.b = b;
    updateSymbol(name, BOOL, value, V_BOOL);
}

Symbol* addSymbolInt(char *name, int i) {
    union Value value;
    value.i = i;
    return addSymbol(name, NUMBER, value, V_INT);
}

void updateSymbolInt(char *name, int i) {
    union Value value;
    value.i = i;
    updateSymbol(name, NUMBER, value, V_INT);
}

void addSymbolFloat(char *name, float f) {
    union Value value;
    value.f = f;
    addSymbol(name, NUMBER, value, V_FLOAT);
}

void updateSymbolFloat(char *name, float f) {
    union Value value;
    value.f = f;
    updateSymbol(name, NUMBER, value, V_FLOAT);
}

Symbol* addSymbolString(char *name, char *s) {
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    free(s);
    return addSymbol(name, STRING, value, V_STRING);
}

void updateSymbolString(char *name, char *s) {
    union Value value;
    value.s = s;
    updateSymbol(name, STRING, value, V_STRING);
}

void addSymbolArray(char *name) {
    union Value value;
    complex_mode = addSymbol(name, ARRAY, value, V_VOID);
}

Symbol* createCloneFromSymbolByName(char *clone_name, enum Type type, char *name, enum Type extra_type) {
    Symbol* symbol = getSymbol(name);
    Symbol* clone_symbol = createCloneFromSymbol(clone_name, type, symbol, extra_type);
    free(name);
    free(clone_name);
    return clone_symbol;
}

Symbol* createCloneFromComplexElement(char *clone_name, enum Type type, char *name, unsigned long long int symbol_id, enum Type extra_type) {
    Symbol* access_symbol = getSymbolById(symbol_id);
    int i = getSymbolValueInt_ZeroIfNotInt(access_symbol);
    char *key = getSymbolValueString_NullIfNotString(access_symbol);
    removeSymbol(access_symbol);

    Symbol* _symbol = getSymbol(name);
    Symbol* symbol = getComplexElement(name, i, key);
    Symbol* clone_symbol = createCloneFromSymbol(clone_name, type, symbol, extra_type);

    free(name);
    free(key);
    free(clone_name);
    if (_symbol->type == STRING) {
        removeSymbol(symbol);
    }

    return clone_symbol;
}

Symbol* createCloneFromSymbol(char *clone_name, enum Type type, Symbol* symbol, enum Type extra_type) {
    if (type != ANY &&
        symbol->type != ANY &&
        symbol->type != type
    ) {
        throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, clone_name);
    }

    Symbol* clone_symbol;
    if (symbol->type == ARRAY || symbol->type == DICT) {
        if (symbol->secondary_type != extra_type) throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, clone_name);
        clone_symbol = deepCopyComplex(clone_name, symbol);
    } else {
        if (type == ANY) {
            clone_symbol = deepCopySymbol(symbol, ANY, NULL);
        } else if (symbol->type == ANY) {
            char *temp_clone_name = malloc(1 + strlen(clone_name));
            strcpy(temp_clone_name, clone_name);
            Symbol* temp_symbol = createSymbolWithoutValueType(temp_clone_name, type);
            clone_symbol = assignByTypeCasting(temp_symbol, symbol);
            removeSymbol(temp_symbol);
        } else {
            clone_symbol = deepCopySymbol(symbol, symbol->type, NULL);
        }
        if (clone_name != NULL) {
            clone_symbol->name = malloc(1 + strlen(clone_name));
            strcpy(clone_symbol->name, clone_name);
        }
    }
    return clone_symbol;
}

Symbol* updateSymbolByClonning(char *clone_name, Symbol* symbol) {
    Symbol* clone_symbol = getSymbol(clone_name);

    if (clone_symbol->type != ANY &&
        symbol->type != ANY &&
        clone_symbol->type != symbol->type
    ) {
        throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, clone_name);
    }

    if (clone_symbol->type == ARRAY) {
        throw_error(E_ARRAYS_ARE_NOT_MASS_ASSIGNABLE, clone_name);
    }

    Symbol* temp_symbol = clone_symbol;

    if (clone_symbol->type == ANY) {
        clone_symbol = deepCopySymbol(symbol, ANY, NULL);
    } else if (symbol->type == ANY) {
        clone_symbol = assignByTypeCasting(clone_symbol, symbol);
    } else {
        clone_symbol = deepCopySymbol(symbol, symbol->type, NULL);
    }
    clone_symbol->name = malloc(1 + strlen(clone_name));
    strcpy(clone_symbol->name, clone_name);

    removeSymbol(temp_symbol);
    return clone_symbol;
}

Symbol* updateSymbolByClonningName(char *clone_name, char *name) {
    Symbol* symbol = getSymbol(name);
    updateSymbolByClonning(clone_name, symbol);
    free(clone_name);
    free(name);
    return symbol;
}

Symbol* updateSymbolByClonningComplexElement(char *clone_name, char *name, unsigned long long int symbol_id) {
    Symbol* access_symbol = getSymbolById(symbol_id);
    int i = getSymbolValueInt_ZeroIfNotInt(access_symbol);
    char *key = getSymbolValueString_NullIfNotString(access_symbol);
    removeSymbol(access_symbol);

    Symbol* _symbol = getSymbol(name);
    Symbol* symbol = getComplexElement(name, i, key);
    updateSymbolByClonning(clone_name, symbol);

    free(clone_name);
    free(name);
    free(key);
    if (_symbol->type == STRING) {
        removeSymbol(symbol);
    }

    return symbol;
}

bool isComplexIllegal(enum Type type) {
    if (complex_mode != NULL && type != ANY) {
        for (int i = 0; i < complex_mode->children_count; i++) {
            Symbol* symbol = complex_mode->children[i];
            if (symbol->type != type) {
                return true;
            }
        }
    }
    return false;
}

void finishComplexMode(char *name, enum Type type) {
    complex_mode->children_count = symbol_counter;
    if (name != NULL) {
        complex_mode->name = malloc(1 + strlen(name));
        strcpy(complex_mode->name, name);
    }
    complex_mode->secondary_type = type;
    if (isComplexIllegal(type)) {
        free(name);
        throw_error(E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_ARRAY, complex_mode->name);
    }
    complex_mode = NULL;
    symbol_counter = 0;
}

Symbol* getArrayElement(char *name, int i) {
    Symbol* symbol = getSymbol(name);
    if (symbol->type == STRING) {
        union Value value;
        value.s = malloc(2 * sizeof(char));
        value.s[0] = symbol->value.s[i];
        value.s[1] = '\0';
        return addSymbol(NULL, STRING, value, V_STRING);
    }

    if (symbol->type != ARRAY) throw_error(E_VARIABLE_IS_NOT_AN_ARRAY, name);

    if (i < 0) {
        i = symbol->children_count + i;
    }

    if (i < 0 || i > symbol->children_count - 1) {
        free(name);
        char buffer[__ITOA_BUFFER_LENGTH__];
        throw_error(E_UNDEFINED_INDEX, itoa(i, buffer, 10));
    }

    return symbol->children[i];
}

void cloneSymbolToComplex(char *name, char *key) {
    Symbol* symbol = getSymbol(name);
    deepCopySymbol(symbol, symbol->type, key);
    free(name);
}

Symbol* getComplexElement(char *name, int i, char *key) {
    Symbol* complex = getSymbol(name);

    Symbol* symbol;
    if (complex->type == ARRAY || complex->type == STRING) {
        symbol = getArrayElement(name, i);
    } else if (complex->type == DICT) {
        symbol = getDictElement(name, key);
    } else {
        free(name);
        throw_error(E_UNRECOGNIZED_COMPLEX_DATA_TYPE, complex->name);
    }

    return symbol;
}

Symbol* getComplexElementBySymbolId(char *name, unsigned long long symbol_id) {
    Symbol* access_symbol = getSymbolById(symbol_id);
    int i = getSymbolValueInt_ZeroIfNotInt(access_symbol);
    char *key = getSymbolValueString_NullIfNotString(access_symbol);
    removeSymbol(access_symbol);

    Symbol* complex = getSymbol(name);

    Symbol* symbol;
    if (complex->type == ARRAY || complex->type == STRING) {
        symbol = getArrayElement(name, i);
    } else if (complex->type == DICT) {
        symbol = getDictElement(name, key);
    } else {
        free(name);
        free(key);
        throw_error(E_UNRECOGNIZED_COMPLEX_DATA_TYPE, complex->name);
    }
    free(key);

    return symbol;
}

void updateComplexElement(char *name, unsigned long long symbol_id, enum Type type, union Value value) {
    Symbol* access_symbol = getSymbolById(symbol_id);
    int i = getSymbolValueInt_ZeroIfNotInt(access_symbol);
    char *key = getSymbolValueString_NullIfNotString(access_symbol);
    removeSymbol(access_symbol);

    Symbol* complex = getSymbol(name);
    if (complex->secondary_type != ANY && complex->secondary_type != type) {
        free(name);
        free(key);
        throw_error(E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_ARRAY, complex->name);
    }

    Symbol* symbol = getComplexElement(name, i, key);
    symbol->type = type;
    if (symbol->value_type == V_STRING) free(symbol->value.s);
    symbol->value = value;

    free(name);
    free(key);
}

void updateComplexElementBool(char* name, unsigned long long symbol_id, bool b) {
    union Value value;
    value.b = b;
    updateComplexElement(name, symbol_id, BOOL, value);
}

void updateComplexElementInt(char* name, unsigned long long symbol_id, int i) {
    union Value value;
    value.i = i;
    updateComplexElement(name, symbol_id, NUMBER, value);
}

void updateComplexElementFloat(char* name, unsigned long long symbol_id, float f) {
    union Value value;
    value.f = f;
    updateComplexElement(name, symbol_id, NUMBER, value);
}

void updateComplexElementString(char* name, unsigned long long symbol_id, char *s) {
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    free(s);
    updateComplexElement(name, symbol_id, STRING, value);
}

void updateComplexElementSymbol(char* name, unsigned long long int symbol_id, char* source_name) {
    Symbol* access_symbol = getSymbolById(symbol_id);
    int i = getSymbolValueInt_ZeroIfNotInt(access_symbol);
    char *key = getSymbolValueString_NullIfNotString(access_symbol);

    Symbol* complex = getSymbol(name);
    Symbol* source = getSymbol(source_name);
    if (complex->secondary_type != ANY && complex->secondary_type != source->type) {
        removeSymbol(access_symbol);
        free(name);
        free(key);
        free(source_name);
        throw_error(E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_ARRAY, complex->name);
    }

    Symbol* symbol;
    if (complex->type == ARRAY) {
        removeSymbol(access_symbol);
        symbol = getArrayElement(name, i);
        removeSymbol(symbol);
        complex->children[i] = deepCopySymbol(source, source->type, NULL);
    } else if (complex->type == DICT) {
        removeComplexElement(name, symbol_id);
        complex_mode = complex;
        symbol_counter = complex->children_count;
        deepCopySymbol(source, source->type, key);
        finishComplexMode(NULL, complex->secondary_type);
    } else {
        removeSymbol(access_symbol);
        free(name);
        free(key);
        free(source_name);
        throw_error(E_UNRECOGNIZED_COMPLEX_DATA_TYPE, complex->name);
    }

    free(name);
    free(source_name);
}

void removeComplexElement(char *name, unsigned long long int symbol_id) {
    Symbol* access_symbol = getSymbolById(symbol_id);
    int i = getSymbolValueInt_ZeroIfNotInt(access_symbol);
    char *key = getSymbolValueString_NullIfNotString(access_symbol);
    removeSymbol(access_symbol);

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
        free(name);
        free(key);
        throw_error(E_UNRECOGNIZED_COMPLEX_DATA_TYPE, name);
    }

    free(key);

    if (i < 0) {
        i = complex->children_count + i;
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

    if (complex->children_count == 0) {
        free(complex->children);
    }
}

void addSymbolDict(char *name) {
    union Value value;
    complex_mode = addSymbol(name, DICT, value, V_VOID);
}

Symbol* getDictElement(char *name, char *key) {
    Symbol* symbol = getSymbol(name);
    if (symbol->type != DICT) {
        free(key);
        throw_error(E_VARIABLE_IS_NOT_A_DICTIONARY, name);
    }

    for (int i = 0; i < symbol->children_count; i++) {
        Symbol* child = symbol->children[i];
        if (child->key != NULL && strcmp(child->key, key) == 0) {
            return child;
        }
    }
    free(name);
    throw_error(E_UNDEFINED_KEY, key);
    return NULL;
}

void addSymbolAnyString(char *name, char *s) {
    union Value value;
    value.s = s;
    addSymbol(name, ANY, value, V_STRING);
}

void addSymbolAnyInt(char *name, int i) {
    union Value value;
    value.i = i;
    addSymbol(name, ANY, value, V_INT);
}

void addSymbolAnyFloat(char *name, float f) {
    union Value value;
    value.f = f;
    addSymbol(name, ANY, value, V_FLOAT);
}

void addSymbolAnyBool(char *name, bool b) {
    union Value value;
    value.b = b;
    addSymbol(name, ANY, value, V_BOOL);
}

_Function* getCurrentScope() {
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
    scope_override = scopeless;
    Symbol* clone_symbol = createCloneFromSymbol(
        NULL,
        symbol->type,
        symbol,
        symbol->secondary_type
    );
    scope_override = NULL;
    return clone_symbol;
}

void freeAllSymbols() {
    symbol_cursor = start_symbol;
    while (symbol_cursor != NULL) {
        Symbol* symbol = symbol_cursor;
        symbol_cursor = symbol_cursor->next;
        freeSymbol(symbol);
    }
}

Symbol* assignByTypeCasting(Symbol* clone_symbol, Symbol* symbol) {
    char buffer[__ITOA_BUFFER_LENGTH__];
    char *val;
    switch (clone_symbol->value_type)
    {
        case V_BOOL:
            switch (symbol->value_type)
            {
                case V_BOOL:
                    clone_symbol->value.b = symbol->value.b;
                    break;
                case V_INT:
                    if (symbol->value.i != 0) {
                        clone_symbol->value.b = true;
                    } else {
                        clone_symbol->value.b = false;
                    }
                    break;
                case V_FLOAT:
                    if (symbol->value.f != 0.0) {
                        clone_symbol->value.b = true;
                    } else {
                        clone_symbol->value.b = false;
                    }
                    break;
                case V_STRING:
                    if (symbol->value.s[0] != '\0') {
                        clone_symbol->value.b = true;
                    } else {
                        clone_symbol->value.b = false;
                    }
                    break;
                default:
                    throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, clone_symbol->name);
                    break;
            }
            break;
        case V_INT:
            switch (symbol->value_type)
            {
                case V_BOOL:
                    clone_symbol->value.i = symbol->value.b ? 1 : 0;
                    break;
                case V_INT:
                    clone_symbol->value.i = symbol->value.i;
                    break;
                case V_FLOAT:
                    clone_symbol->value.i = (int)symbol->value.f;
                    break;
                case V_STRING:
                    clone_symbol->value.i = atoi(symbol->value.s);
                    break;
                default:
                    throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, clone_symbol->name);
                    break;
            }
            break;
        case V_FLOAT:
            switch (symbol->value_type)
            {
                case V_BOOL:
                    clone_symbol->value.f = symbol->value.b ? 1.0 : 0.0;
                    break;
                case V_INT:
                    clone_symbol->value.f = (float)symbol->value.i;
                    break;
                case V_FLOAT:
                    clone_symbol->value.f = symbol->value.f;
                    break;
                case V_STRING:
                    clone_symbol->value.f = atof(symbol->value.s);
                    break;
                default:
                    throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, clone_symbol->name);
                    break;
            }
            break;
        case V_STRING:
            switch (symbol->value_type)
            {
                case V_BOOL:
                    free(clone_symbol->value.s);
                    val = symbol->value.b ? "true" : "false";
                    clone_symbol->value.s = malloc(1 + strlen(val));
                    strcpy(clone_symbol->value.s, val);
                    break;
                case V_INT:
                    free(clone_symbol->value.s);
                    val = itoa(symbol->value.i, buffer, 10);
                    clone_symbol->value.s = malloc(1 + strlen(val));
                    strcpy(clone_symbol->value.s, val);
                    break;
                case V_FLOAT:
                    free(clone_symbol->value.s);
                    sprintf(buffer, "%g", symbol->value.f);
                    val = buffer;
                    clone_symbol->value.s = malloc(1 + strlen(val));
                    strcpy(clone_symbol->value.s, val);
                    break;
                case V_STRING:
                    free(clone_symbol->value.s);
                    clone_symbol->value.s = malloc(1 + strlen(symbol->value.s));
                    strcpy(clone_symbol->value.s, symbol->value.s);
                    break;
                default:
                    throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, clone_symbol->name);
                    break;
            }
            break;
        default:
            throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, clone_symbol->name);
            break;
    }
    return deepCopySymbol(clone_symbol, clone_symbol->type, NULL);
}

Symbol* createSymbolWithoutValueType(char *name, enum Type type) {
    union Value value;
    enum ValueType value_type;

    switch (type)
    {
        case BOOL:
            value_type = V_BOOL;
            break;
        case NUMBER:
            value_type = V_FLOAT;
            break;
        case STRING:
            value.s = malloc(1 + strlen(""));
            strcpy(value.s, "");
            value_type = V_STRING;
            break;
        default:
            throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, name);
            break;
    }

    return addSymbol(name, type, value, value_type);
}

void removeSymbolsByScope(_Function* scope) {
    symbol_cursor = start_symbol;
    while (symbol_cursor != NULL) {
        Symbol* symbol = symbol_cursor;
        symbol_cursor = symbol_cursor->next;
        if (strcmp(symbol->scope->name, scope->name) == 0) {
            removeSymbol(symbol);
        }
    }
}

int incrementThenAssign(char *name, int i) {
    char *name1 = malloc(1 + strlen(name));
    strcpy(name1, name);
    char *name2 = malloc(1 + strlen(name));
    strcpy(name2, name);

    updateSymbolInt(name2, getSymbolValueInt(name1) + i);
    return getSymbolValueInt(name);
}

int assignThenIncrement(char *name, int i) {
    char *name1 = malloc(1 + strlen(name));
    strcpy(name1, name);
    char *name2 = malloc(1 + strlen(name));
    strcpy(name2, name);

    int result = getSymbolValueInt(name);
    updateSymbolInt(name2, getSymbolValueInt(name1) + i);
    return result;
}
