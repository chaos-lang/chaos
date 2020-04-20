#include "symbol.h"

char *type_names[] = {
    "Boolean",
    "Number",
    "String",
    "Any",
    "Array",
    "Dictionary",
    "Void"
};

char *value_type_names[] = {
    "Boolean",
    "Number",
    "Number",
    "String",
    "Void"
};

unsigned long long symbol_id_counter = 0;
bool disable_complex_mode = false;

Symbol* addSymbol(char *name, enum Type type, union Value value, enum ValueType value_type) {
    symbol_cursor = start_symbol;

    Symbol* symbol;
    symbol = (struct Symbol*)calloc(1, sizeof(Symbol));
    symbol->id = symbol_id_counter++;

    if (isComplexMode() && complex_mode_stack.arr[complex_mode_stack.size - 1]->type == K_DICT) {
        if (name != NULL) {
            symbol->key = malloc(1 + strlen(name));
            strcpy(symbol->key, name);
        }
    } else {
        if (isDefined(name)) {
            free(symbol);
            if (type == K_STRING) {
                free(value.s);
            }
            append_to_array_without_malloc(&free_string_stack, name);
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

    if (symbol->type != K_ANY && symbol->type != type) {
        append_to_array_without_malloc(&free_string_stack, name);
        throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, getTypeName(type), name);
    }

    if (symbol->value_type == V_STRING) free(symbol->value.s);
    symbol->value = value;
    symbol->value_type = value_type;

    free(name);

    return symbol;
}

void removeSymbolByName(char *name) {
    Symbol* symbol = getSymbol(name);
    removeSymbol(symbol);
}

void removeSymbol(Symbol* symbol) {
    removeChildrenOfComplex(symbol);

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
    append_to_array_without_malloc(&free_string_stack, name);
    throw_error(E_UNDEFINED_VARIABLE, name);
    return NULL;
}

Symbol* getSymbolById(unsigned long long id) {
    symbol_cursor = start_symbol;
    while (symbol_cursor != NULL) {
        if (symbol_cursor->id == id) {
            Symbol* symbol = symbol_cursor;
            return symbol;
        }
        symbol_cursor = symbol_cursor->next;
    }
    throw_error(E_NO_VARIABLE_WITH_ID, NULL, NULL, 0, id);
    return NULL;
}

Symbol* deepCopySymbol(Symbol* symbol, enum Type type, char *key) {
    Symbol* clone_symbol;
    if (type == K_ARRAY || type == K_DICT) {
        clone_symbol = deepCopyComplex(NULL, symbol);
        clone_symbol->key = key;
    } else {
        clone_symbol = addSymbol(key, type, symbol->value, symbol->value_type);
    }
    if (symbol->value_type == V_STRING) {
        clone_symbol->value.s = malloc(1 + strlen(symbol->value.s));
        strcpy(clone_symbol->value.s, symbol->value.s);
    }
    return clone_symbol;
}

Symbol* deepCopyComplex(char *name, Symbol* symbol) {
    if (symbol->type == K_ARRAY) {
        addSymbolArray(NULL);
    } else if (symbol->type == K_DICT) {
        addSymbolDict(NULL);
    } else {
        append_to_array_without_malloc(&free_string_stack, name);
        throw_error(E_UNRECOGNIZED_COMPLEX_DATA_TYPE, getTypeName(symbol->type), name);
    }

    for (unsigned long i = 0; i < symbol->children_count; i++) {
        Symbol* child = symbol->children[i];
        char *key = NULL;
        if (child->key != NULL) {
            key = malloc(1 + strlen(child->key));
            strcpy(key, child->key);
        }
        deepCopySymbol(child, child->type, key);
    }

    Symbol* symbol_return = complex_mode_stack.arr[complex_mode_stack.size - 1];
    finishComplexMode(name, K_ANY);

    return symbol_return;
}

char* getSymbolValueString(char *name) {
    Symbol* symbol = getSymbol(name);
    free(name);
    char* value;
    if (symbol->value_type == V_STRING) {
        value = malloc(1 + strlen(symbol->value.s));
        strcpy(value, symbol->value.s);
        return value;
    } else {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return "";
}

long double getSymbolValueFloat(char *name) {
    Symbol* symbol = getSymbol(name);
    free(name);
    long double value;
    switch (symbol->value_type)
    {
        case V_BOOL:
            value = symbol->value.b ? 1.0 : 0.0;
            return value;
            break;
        case V_INT:
            value = (long double)symbol->value.i;
            return value;
            break;
        case V_FLOAT:
            value = symbol->value.f;
            return value;
            break;
        default:
            throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
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

long long getSymbolValueInt(char *name) {
    Symbol* symbol = getSymbol(name);
    free(name);
    long long value;
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
            value = (long long)symbol->value.f;
            return value;
            break;
        default:
            throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
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

long long getSymbolValueInt_ZeroIfNotInt(Symbol* symbol) {
    if (symbol->value_type != V_INT && symbol->value_type != V_FLOAT) {
        return 0;
    }
    long long value;
    switch (symbol->value_type)
    {
        case V_BOOL:
            value = symbol->value.b ? 1 : 0;
            break;
        case V_INT:
            value = symbol->value.i;
            break;
        case V_FLOAT:
            value = (long long)symbol->value.f;
            break;
        default:
            throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
            break;
    }
    return value * symbol->sign;
}

void printSymbolValue(Symbol* symbol, bool is_complex, bool pretty, unsigned long iter) {
    switch (symbol->type)
    {
        case K_BOOL:
            printf("%s", symbol->value.b ? "true" : "false");
            break;
        case K_NUMBER:
            switch (symbol->value_type)
            {
                case V_INT:
                    printf("%lld", symbol->value.i);
                    break;
                case V_FLOAT:
                    printf("%Lg", symbol->value.f);
                    break;
                default:
                    throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
                    break;
            }
            break;
        case K_STRING:
            if (is_complex) {
                printf("'%s'", symbol->value.s);
            } else {
                printf("%s", symbol->value.s);
            }
            break;
        case K_ARRAY:
            iter++;
            printf("[");
            if (pretty) {
                printf("\n");
            }
            for (unsigned long i = 0; i < symbol->children_count; i++) {
                if (pretty) {
                    for (unsigned long j = 0; j < iter; j++) {
                        printf(__KAOS_TAB__);
                    }
                }
                printSymbolValue(symbol->children[i], true, pretty, iter);
                if (i + 1 != symbol->children_count) {
                    if (pretty) {
                        printf(",\n");
                    } else {
                        printf(", ");
                    }
                }
            }
            if (pretty) {
                printf("\n");
            }
            if (pretty) {
                for (unsigned long j = 0; j < (iter - 1); j++) {
                    printf(__KAOS_TAB__);
                }
            }
            printf("]");
            break;
        case K_DICT:
            iter++;
            printf("{");
            if (pretty) {
                printf("\n");
            }
            for (unsigned long i = 0; i < symbol->children_count; i++) {
                if (pretty) {
                    for (unsigned long j = 0; j < iter; j++) {
                        printf(__KAOS_TAB__);
                    }
                }
                Symbol* child = symbol->children[i];
                printf("'%s': ", child->key);
                printSymbolValue(child, true, pretty, iter);
                if (i + 1 != symbol->children_count) {
                    if (pretty) {
                        printf(",\n");
                    } else {
                        printf(", ");
                    }
                }
            }
            if (pretty) {
                printf("\n");
            }
            if (pretty) {
                for (unsigned long j = 0; j < (iter - 1); j++) {
                    printf(__KAOS_TAB__);
                }
            }
            printf("}");
            break;
        case K_ANY:
            switch (symbol->value_type)
            {
                case V_STRING:
                    printf("%s", symbol->value.s);
                    break;
                case V_INT:
                    printf("%lld", symbol->value.i);
                    break;
                case V_FLOAT:
                    printf("%Lg", symbol->value.f);
                    break;
                case V_BOOL:
                    printf("%s", symbol->value.b ? "true" : "false");
                    break;
                default:
                    throw_error(E_UNEXPECTED_VALUE_TYPE, getTypeName(symbol->value_type), symbol->name);
                    break;
            }
            break;
        default:
            throw_error(E_UNKNOWN_VARIABLE_TYPE, getTypeName(symbol->type), symbol->name);
            break;
    }
}

void printSymbolValueEndWith(Symbol* symbol, char *end, bool pretty) {
    printSymbolValue(symbol, false, pretty, 0);
    printf("%s", end);
}

void printSymbolValueEndWithNewLine(Symbol* symbol, bool pretty) {
    printSymbolValueEndWith(symbol, "\n", pretty);
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
    if (!isComplexMode()) return;

    complex_mode_stack.arr[complex_mode_stack.size - 1]->children = realloc(
        complex_mode_stack.arr[complex_mode_stack.size - 1]->children,
        sizeof(Symbol) * ++complex_mode_stack.child_counter[complex_mode_stack.size - 1]
    );

    if (complex_mode_stack.arr[complex_mode_stack.size - 1]->children == NULL) {
        throw_error(E_MEMORY_ALLOCATION_FOR_ARRAY_FAILED, complex_mode_stack.arr[complex_mode_stack.size - 1]->name);
    }

    complex_mode_stack.arr[complex_mode_stack.size - 1]->children[
        complex_mode_stack.child_counter[complex_mode_stack.size - 1] - 1
    ] = symbol;
}

void printSymbolTable() {
    Symbol *symbol = start_symbol;
    printf("[start] =>\n");
    while(symbol != NULL) {
        _Function* scope = symbol->scope;
        printf(
            "\t{id: %llu, name: %s, 2nd_name: %s, key: %s, scope: %s, depth: %hu, type: %u, 2nd_type: %u, value_type: %u, role: %u, param_of: %s} =>\n",
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

Symbol* addSymbolBool(char *name, bool b) {
    union Value value;
    value.b = b;
    return addSymbol(name, K_BOOL, value, V_BOOL);
}

void updateSymbolBool(char *name, bool b) {
    union Value value;
    value.b = b;
    updateSymbol(name, K_BOOL, value, V_BOOL);
}

Symbol* addSymbolInt(char *name, long long i) {
    union Value value;
    value.i = i;
    return addSymbol(name, K_NUMBER, value, V_INT);
}

void updateSymbolInt(char *name, long long i) {
    union Value value;
    value.i = i;
    updateSymbol(name, K_NUMBER, value, V_INT);
}

Symbol* addSymbolFloat(char *name, long double f) {
    union Value value;
    value.f = f;
    return addSymbol(name, K_NUMBER, value, V_FLOAT);
}

void updateSymbolFloat(char *name, long double f) {
    union Value value;
    value.f = f;
    updateSymbol(name, K_NUMBER, value, V_FLOAT);
}

Symbol* addSymbolString(char *name, char *s) {
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    free(s);
    return addSymbol(name, K_STRING, value, V_STRING);
}

void updateSymbolString(char *name, char *s) {
    union Value value;
    value.s = s;
    updateSymbol(name, K_STRING, value, V_STRING);
}

void addSymbolArray(char *name) {
    union Value value;
    Symbol* complex_mode = addSymbol(name, K_ARRAY, value, V_VOID);
    pushComplexModeStack(complex_mode);
}

Symbol* createCloneFromSymbolByName(char *clone_name, enum Type type, char *name, enum Type extra_type) {
    Symbol* symbol = getSymbol(name);
    Symbol* clone_symbol = createCloneFromSymbol(clone_name, type, symbol, extra_type);
    free(name);
    free(clone_name);
    return clone_symbol;
}

Symbol* createCloneFromComplexElement(char *clone_name, enum Type type, char *name, enum Type extra_type) {
    Symbol* _symbol = getComplexElementThroughLeftRightBracketStack(name, 1);
    unsigned long long symbol_id = popLeftRightBracketStack();

    Symbol* access_symbol = getSymbolById(symbol_id);
    long long i = getSymbolValueInt_ZeroIfNotInt(access_symbol);
    char *key = getSymbolValueString_NullIfNotString(access_symbol);
    removeSymbol(access_symbol);

    Symbol* symbol = getComplexElement(_symbol, i, key);
    Symbol* clone_symbol = createCloneFromSymbol(clone_name, type, symbol, extra_type);

    free(key);
    free(clone_name);
    if (_symbol->type == K_STRING) {
        removeSymbol(symbol);
    }

    return clone_symbol;
}

Symbol* createCloneFromSymbol(char *clone_name, enum Type type, Symbol* symbol, enum Type extra_type) {
    if (type != K_ANY &&
        symbol->type != K_ANY &&
        symbol->type != type
    ) {
        append_to_array_without_malloc(&free_string_stack, clone_name);
        throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, getTypeName(type), clone_name);
    }

    Symbol* clone_symbol;
    if (symbol->type == K_ARRAY || symbol->type == K_DICT) {
        if (symbol->secondary_type != extra_type) {
            append_to_array_without_malloc(&free_string_stack, clone_name);
            throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, getTypeName(extra_type), clone_name);
        }
        clone_symbol = deepCopyComplex(clone_name, symbol);
    } else {
        if (type == K_ANY) {
            clone_symbol = deepCopySymbol(symbol, K_ANY, NULL);
        } else if (symbol->type == K_ANY) {
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

    if (clone_symbol->type != K_ANY &&
        symbol->type != K_ANY &&
        clone_symbol->type != symbol->type
    ) {
        append_to_array_without_malloc(&free_string_stack, clone_name);
        throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, getTypeName(symbol->type), clone_name);
    }

    if (clone_symbol->type == K_ARRAY) {
        append_to_array_without_malloc(&free_string_stack, clone_name);
        throw_error(E_ARRAYS_ARE_NOT_MASS_ASSIGNABLE, clone_name);
    }

    Symbol* temp_symbol = clone_symbol;

    if (clone_symbol->type == K_ANY) {
        clone_symbol = deepCopySymbol(symbol, K_ANY, NULL);
    } else if (symbol->type == K_ANY) {
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

Symbol* updateSymbolByClonningComplexElement(char *clone_name, char *name) {
    Symbol* _symbol = getComplexElementThroughLeftRightBracketStack(name, 1);
    unsigned long long symbol_id = popLeftRightBracketStack();

    Symbol* access_symbol = getSymbolById(symbol_id);
    long long i = getSymbolValueInt_ZeroIfNotInt(access_symbol);
    char *key = getSymbolValueString_NullIfNotString(access_symbol);
    removeSymbol(access_symbol);

    Symbol* symbol = getComplexElement(_symbol, i, key);
    updateSymbolByClonning(clone_name, symbol);

    free(clone_name);
    free(key);
    if (_symbol->type == K_STRING) {
        removeSymbol(symbol);
    }

    return symbol;
}

enum Type isComplexIllegal(enum Type type) {
    if (isComplexMode() && type != K_ANY) {
        for (unsigned long i = 0; i < complex_mode_stack.arr[complex_mode_stack.size - 1]->children_count; i++) {
            Symbol* symbol = complex_mode_stack.arr[complex_mode_stack.size - 1]->children[i];
            if (symbol->type != type) {
                return symbol->type;
            }
        }
    }
    return -1;
}

Symbol* finishComplexMode(char *name, enum Type type) {
    Symbol* complex_mode = getComplexMode();
    complex_mode->children_count = complex_mode_stack.child_counter[complex_mode_stack.size - 1];
    if (name != NULL) {
        if (isDefined(name)) {
            removeSymbol(complex_mode);
            popComplexModeStack();
            append_to_array_without_malloc(&free_string_stack, name);
            throw_error(E_VARIABLE_ALREADY_DEFINED, name);
        }

        complex_mode->name = malloc(1 + strlen(name));
        strcpy(complex_mode->name, name);
    }
    complex_mode->secondary_type = type;
    enum Type illegal_type = isComplexIllegal(type);
    if (illegal_type != -1) {
        if (name != NULL)
            free(name);
        throw_error(E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_ARRAY, getTypeName(illegal_type), complex_mode->name);
    }
    popComplexModeStack();
    return complex_mode;
}

void finishComplexModeWithUpdate(char *name) {
    Symbol* symbol = getSymbol(name);
    _finishComplexModeWithUpdate(symbol);
}

void _finishComplexModeWithUpdate(Symbol* symbol) {
    enum Type type;
    if (isComplex(symbol)) {
        type = symbol->secondary_type;
    } else {
        type = K_ANY;
    }

    Symbol* complex_mode = finishComplexMode(NULL, type);

    if (isComplex(symbol)) {
        removeChildrenOfComplex(symbol);
    }

    symbol->children = realloc(
        symbol->children,
        sizeof(Symbol) * complex_mode->children_count
    );
    memcpy(symbol->children, complex_mode->children, complex_mode->children_count * sizeof(Symbol));

    symbol->children_count = complex_mode->children_count;
    symbol->type = complex_mode->type;
    symbol->secondary_type = complex_mode->secondary_type;
    if (symbol->value_type == V_STRING) free(symbol->value.s);
    symbol->value_type = complex_mode->value_type;
    symbol->value = complex_mode->value;

    free(complex_mode->children);
    complex_mode->children = NULL;
    complex_mode->children_count = 0;
    removeSymbol(complex_mode);
}

Symbol* getArrayElement(Symbol* symbol, long long i) {
    long long orig_i = i;
    if (symbol->type == K_STRING) {
        union Value value;
        value.s = malloc(2 * sizeof(char));

        if (i < 0) {
            i = strlen(symbol->value.s) + i;
        }

        if (i < 0 || i > strlen(symbol->value.s) - 1) {
            free(value.s);
            throw_error(E_INDEX_OUT_OF_RANGE_STRING, symbol->name, NULL, orig_i);
        }

        value.s[0] = symbol->value.s[i];
        value.s[1] = '\0';
        return addSymbol(NULL, K_STRING, value, V_STRING);
    }

    if (symbol->type != K_ARRAY) throw_error(E_VARIABLE_IS_NOT_AN_ARRAY, symbol->name);

    if (i < 0) {
        i = (long long) symbol->children_count + i;
    }

    if (i < 0 || i > (long long) symbol->children_count - 1) {
        throw_error(E_INDEX_OUT_OF_RANGE, symbol->name, NULL, orig_i);
    }

    return symbol->children[i];
}

void cloneSymbolToComplex(char *name, char *key) {
    Symbol* symbol = getSymbol(name);
    Symbol* cloned_symbol = deepCopySymbol(symbol, symbol->type, key);
    free(name);
}

Symbol* getComplexElement(Symbol* complex, long long i, char *key) {

    Symbol* symbol;
    if (complex->type == K_ARRAY || complex->type == K_STRING) {
        symbol = getArrayElement(complex, i);
    } else if (complex->type == K_DICT) {
        symbol = getDictElement(complex, key);
    } else {
        throw_error(E_UNRECOGNIZED_COMPLEX_DATA_TYPE, getTypeName(complex->type), complex->name);
    }

    return symbol;
}

Symbol* getComplexElementBySymbolId(Symbol* complex, unsigned long long symbol_id) {
    Symbol* access_symbol = getSymbolById(symbol_id);
    long long i = getSymbolValueInt_ZeroIfNotInt(access_symbol);
    char *key = getSymbolValueString_NullIfNotString(access_symbol);
    removeSymbol(access_symbol);

    Symbol* symbol;
    if (complex->type == K_ARRAY || complex->type == K_STRING) {
        symbol = getArrayElement(complex, i);
    } else if (complex->type == K_DICT) {
        symbol = getDictElement(complex, key);
    } else {
        free(key);
        throw_error(E_UNRECOGNIZED_COMPLEX_DATA_TYPE, getTypeName(complex->type), complex->name);
    }
    free(key);

    return symbol;
}

void updateComplexElementComplex() {
    Symbol* complex = variable_complex_element;
    complex = getComplexElementBySymbolId(complex, variable_complex_element_symbol_id);
    _finishComplexModeWithUpdate(complex);
}

void updateComplexElementWrapper(enum Type type, union Value value, enum ValueType value_type) {
    Symbol* complex = variable_complex_element;
    unsigned long long symbol_id = variable_complex_element_symbol_id;
    updateComplexElement(complex, symbol_id, type, value, value_type);
}

void updateComplexElement(Symbol* complex, unsigned long long symbol_id, enum Type type, union Value value, enum ValueType value_type) {
    Symbol* access_symbol = getSymbolById(symbol_id);
    long long i = getSymbolValueInt_ZeroIfNotInt(access_symbol);
    char *key = getSymbolValueString_NullIfNotString(access_symbol);
    removeSymbol(access_symbol);

    if (complex->type == K_STRING) {
        if (type != K_STRING) {
            free(key);
            throw_error(E_ILLEGAL_CHARACTER_ASSIGNMENT_FOR_STRING, complex->name);
        }

        if (strlen(value.s) > 1) {
            free(key);
            throw_error(E_NOT_A_CHARACTER, complex->name);
        }

        long long orig_i = i;

        if (i < 0) {
            i = strlen(complex->value.s) + i;
        }

        if (i < 0 || i > strlen(complex->value.s) - 1) {
            char buffer[__KAOS_ITOA_BUFFER_LENGTH__];
            throw_error(E_INDEX_OUT_OF_RANGE_STRING, complex->name, NULL, orig_i);
        }

        complex->value.s[i] = value.s[0];
        free(value.s);
        return;
    } else if (complex->secondary_type != K_ANY && complex->secondary_type != type) {
        free(key);
        throw_error(E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_ARRAY, getTypeName(type), complex->name);
    }

    Symbol* symbol = getComplexElement(complex, i, key);
    removeChildrenOfComplex(symbol);
    symbol->type = type;
    if (symbol->value_type == V_STRING) free(symbol->value.s);
    symbol->value_type = value_type;
    symbol->value = value;

    free(key);
}

void updateComplexElementBool(bool b) {
    union Value value;
    value.b = b;
    updateComplexElementWrapper(K_BOOL, value, V_BOOL);
}

void updateComplexElementInt(long long i) {
    union Value value;
    value.i = i;
    updateComplexElementWrapper(K_NUMBER, value, V_INT);
}

void updateComplexElementFloat(long double f) {
    union Value value;
    value.f = f;
    updateComplexElementWrapper(K_NUMBER, value, V_FLOAT);
}

void updateComplexElementString(char *s) {
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    free(s);
    updateComplexElementWrapper(K_STRING, value, V_STRING);
}

void updateComplexElementSymbol(Symbol* source) {
    Symbol* complex = variable_complex_element;
    unsigned long long symbol_id = variable_complex_element_symbol_id;

    _updateComplexElementSymbol(complex, symbol_id, source);
}

void _updateComplexElementSymbol(Symbol* complex, unsigned long long symbol_id, Symbol* source) {
    Symbol* access_symbol = getSymbolById(symbol_id);
    long long i = getSymbolValueInt_ZeroIfNotInt(access_symbol);
    char *key = getSymbolValueString_NullIfNotString(access_symbol);

    if (complex->secondary_type != K_ANY && complex->secondary_type != source->type) {
        removeSymbol(access_symbol);
        free(key);
        throw_error(E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_ARRAY, getTypeName(source->type), complex->name);
    }

    Symbol* symbol;
    if (complex->type == K_ARRAY) {
        removeSymbol(access_symbol);
        symbol = getArrayElement(complex, i);
        removeSymbol(symbol);
        complex->children[i] = deepCopySymbol(source, source->type, NULL);
    } else if (complex->type == K_DICT) {
        removeComplexElement(complex, symbol_id);
        pushComplexModeStack(complex);
        complex_mode_stack.child_counter[complex_mode_stack.size - 1] = complex->children_count;
        deepCopySymbol(source, source->type, key);
        finishComplexMode(NULL, complex->secondary_type);
    } else {
        removeSymbol(access_symbol);
        free(key);
        throw_error(E_UNRECOGNIZED_COMPLEX_DATA_TYPE, getTypeName(complex->type), complex->name);
    }
}

void removeComplexElement(Symbol* complex, unsigned long long symbol_id) {
    Symbol* access_symbol = getSymbolById(symbol_id);
    long long i = getSymbolValueInt_ZeroIfNotInt(access_symbol);
    char *key = getSymbolValueString_NullIfNotString(access_symbol);
    removeSymbol(access_symbol);

    Symbol* symbol;
    if (complex->type == K_STRING) {
        long long orig_i = i;

        if (i < 0) {
            i = strlen(complex->value.s) + i;
        }

        if (i < 0 || i > strlen(complex->value.s) - 1) {
            throw_error(E_INDEX_OUT_OF_RANGE_STRING, complex->name, NULL, orig_i);
        }

        memmove(&complex->value.s[i], &complex->value.s[i + 1], strlen(complex->value.s) - i);
        return;
    } else if (complex->type == K_ARRAY) {
        symbol = getArrayElement(complex, i);
    } else if (complex->type == K_DICT) {
        symbol = getDictElement(complex, key);
        for (unsigned long j = 0; j < complex->children_count; j++) {
            Symbol* child = complex->children[j];
            if (strcmp(child->key, key) == 0) {
                i = j;
            }
        }
    } else {
        free(key);
        throw_error(E_UNRECOGNIZED_COMPLEX_DATA_TYPE, getTypeName(complex->type), complex->name);
    }

    free(key);

    if (i < 0) {
        i = (long long) complex->children_count + i;
    }

    Symbol** temp = malloc((complex->children_count - 1) * sizeof(Symbol));

    // Copy everything before the index
    if (i != 0)
        memcpy(temp, complex->children, i * sizeof(Symbol));

    // Copy everything after the index
    if (i != (long long) complex->children_count - 1)
        memcpy(temp + i, complex->children + i + 1, (complex->children_count - i - 1) * sizeof(Symbol));

    free(complex->children);
    complex->children = temp;
    removeSymbol(symbol);
    complex->children_count--;

    if (complex->children_count == 0) {
        free(complex->children);
    }
}

void removeComplexElementByLeftRightBracketStack(char *name) {
    Symbol* complex = getComplexElementThroughLeftRightBracketStack(name, 1);
    removeComplexElement(complex, popLeftRightBracketStack());
}

void addSymbolDict(char *name) {
    union Value value;
    Symbol* complex_mode = addSymbol(name, K_DICT, value, V_VOID);
    pushComplexModeStack(complex_mode);
}

Symbol* getDictElement(Symbol* symbol, char *key) {
    if (symbol->type != K_DICT) {
        free(key);
        throw_error(E_VARIABLE_IS_NOT_A_DICTIONARY, symbol->name);
    }

    for (unsigned long i = 0; i < symbol->children_count; i++) {
        Symbol* child = symbol->children[i];
        if (child->key != NULL && strcmp(child->key, key) == 0) {
            return child;
        }
    }
    append_to_array_without_malloc(&free_string_stack, key);
    throw_error(E_UNDEFINED_KEY, key, symbol->name);
    return NULL;
}

void addSymbolAnyString(char *name, char *s) {
    union Value value;
    value.s = s;
    addSymbol(name, K_ANY, value, V_STRING);
}

void addSymbolAnyInt(char *name, long long i) {
    union Value value;
    value.i = i;
    addSymbol(name, K_ANY, value, V_INT);
}

void addSymbolAnyFloat(char *name, long double f) {
    union Value value;
    value.f = f;
    addSymbol(name, K_ANY, value, V_FLOAT);
}

void addSymbolAnyBool(char *name, bool b) {
    union Value value;
    value.b = b;
    addSymbol(name, K_ANY, value, V_BOOL);
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
    char buffer[__KAOS_ITOA_BUFFER_LENGTH__];
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
                    throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), clone_symbol->name);
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
                    clone_symbol->value.i = (long long)symbol->value.f;
                    break;
                case V_STRING:
                    clone_symbol->value.i = atoi(symbol->value.s);
                    break;
                default:
                    throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), clone_symbol->name);
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
                    clone_symbol->value.f = (long double)symbol->value.i;
                    break;
                case V_FLOAT:
                    clone_symbol->value.f = symbol->value.f;
                    break;
                case V_STRING:
                    clone_symbol->value.f = atof(symbol->value.s);
                    break;
                default:
                    throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), clone_symbol->name);
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
                    val = longlong_to_string(symbol->value.i, buffer, 10);
                    clone_symbol->value.s = malloc(1 + strlen(val));
                    strcpy(clone_symbol->value.s, val);
                    break;
                case V_FLOAT:
                    free(clone_symbol->value.s);
                    sprintf(buffer, "%Lg", symbol->value.f);
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
                    throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), clone_symbol->name);
                    break;
            }
            break;
        default:
            throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, getTypeName(symbol->type), clone_symbol->name);
            break;
    }
    return deepCopySymbol(clone_symbol, clone_symbol->type, NULL);
}

Symbol* createSymbolWithoutValueType(char *name, enum Type type) {
    union Value value;
    enum ValueType value_type;

    switch (type)
    {
        case K_BOOL:
            value_type = V_BOOL;
            break;
        case K_NUMBER:
            value_type = V_FLOAT;
            break;
        case K_STRING:
            value.s = malloc(1 + strlen(""));
            strcpy(value.s, "");
            value_type = V_STRING;
            break;
        default:
            append_to_array_without_malloc(&free_string_stack, name);
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
            return removeSymbolsByScope(scope);
        }
    }
}

long long incrementThenAssign(char *name, long long i) {
    char *name1 = malloc(1 + strlen(name));
    strcpy(name1, name);
    char *name2 = malloc(1 + strlen(name));
    strcpy(name2, name);

    updateSymbolInt(name2, getSymbolValueInt(name1) + i);
    return getSymbolValueInt(name);
}

long long assignThenIncrement(char *name, long long i) {
    char *name1 = malloc(1 + strlen(name));
    strcpy(name1, name);
    char *name2 = malloc(1 + strlen(name));
    strcpy(name2, name);

    long long result = getSymbolValueInt(name);
    updateSymbolInt(name2, getSymbolValueInt(name1) + i);
    return result;
}

char* getTypeName(unsigned i) {
    char *name = malloc(1 + strlen(type_names[i]));
    strcpy(name, type_names[i]);
    append_to_array_without_malloc(&free_string_stack, name);
    return name;
}

char* getValueTypeName(unsigned i) {
    char *name = malloc(1 + strlen(value_type_names[i]));
    strcpy(name, value_type_names[i]);
    append_to_array_without_malloc(&free_string_stack, name);
    return name;
}

void pushComplexModeStack(Symbol* complex_mode) {
    if (complex_mode_stack.capacity == 0) {
        complex_mode_stack.arr = (Symbol**)malloc((complex_mode_stack.capacity = 2) * sizeof(Symbol));
        complex_mode_stack.child_counter = (unsigned long*)malloc((complex_mode_stack.capacity = 2) * sizeof(unsigned long*));
    } else if (complex_mode_stack.capacity == complex_mode_stack.size) {
        complex_mode_stack.arr = (Symbol**)realloc(complex_mode_stack.arr, (complex_mode_stack.capacity *= 2) * sizeof(Symbol));
        complex_mode_stack.child_counter = (unsigned long*)realloc(complex_mode_stack.child_counter, (complex_mode_stack.capacity *= 2) * sizeof(unsigned long*));
    }

    complex_mode_stack.arr[complex_mode_stack.size] = complex_mode;
    complex_mode_stack.child_counter[complex_mode_stack.size] = 0;
    complex_mode_stack.size++;
}

void popComplexModeStack() {
    complex_mode_stack.arr[complex_mode_stack.size - 1] = NULL;
    complex_mode_stack.child_counter[complex_mode_stack.size - 1] = 0;
    complex_mode_stack.size--;
}

void freeComplexModeStack() {
    for (unsigned i = 0; i < complex_mode_stack.size; i++) {
        if (complex_mode_stack.arr[i] != NULL)
            removeSymbol(complex_mode_stack.arr[i]);
            complex_mode_stack.child_counter[i] = 0;
    }

    if (complex_mode_stack.capacity > 0) {
        free(complex_mode_stack.arr);
        free(complex_mode_stack.child_counter);
    }

    complex_mode_stack.capacity = 0;
    complex_mode_stack.size = 0;
}

bool isComplexMode() {
    if (disable_complex_mode) {
        return false;
    }
    return complex_mode_stack.size > 0;
}

bool isNestedComplexMode() {
    return complex_mode_stack.size > 1;
}

Symbol* getComplexMode() {
    return complex_mode_stack.arr[complex_mode_stack.size - 1];
}

void pushLeftRightBracketStack(unsigned long long symbol_id) {
    // This function rather prepends the stack
    if (left_right_bracket_stack.capacity == 0) {
        left_right_bracket_stack.arr = (unsigned long long*)malloc((left_right_bracket_stack.capacity += 1) * sizeof(unsigned long long*));
        left_right_bracket_stack.arr[0] = symbol_id;
        left_right_bracket_stack.size++;
        return;
    } else {
        left_right_bracket_stack.arr = (unsigned long long*)realloc(left_right_bracket_stack.arr, (left_right_bracket_stack.capacity += 1) * sizeof(unsigned long long*));
    }

    for (unsigned k = left_right_bracket_stack.size; k > 0; k--) {
        left_right_bracket_stack.arr[k] = left_right_bracket_stack.arr[k - 1];
    }

    left_right_bracket_stack.arr[0] = symbol_id;
    left_right_bracket_stack.size++;
}

unsigned long long popLeftRightBracketStack() {
    unsigned long long symbol_id = left_right_bracket_stack.arr[left_right_bracket_stack.size - 1];
    left_right_bracket_stack.size--;
    return symbol_id;
}

void freeLeftRightBracketStack() {
    free(left_right_bracket_stack.arr);
    left_right_bracket_stack.capacity = 0;
    left_right_bracket_stack.size = 0;
}

Symbol* getComplexElementThroughLeftRightBracketStack(char *name, unsigned long inverse_nested) {
    Symbol* symbol = getSymbol(name);
    free(name);

    if (inverse_nested >= left_right_bracket_stack.size) {
        return symbol;
    }

    unsigned long long symbol_id = popLeftRightBracketStack();
    symbol = getComplexElementBySymbolId(symbol, symbol_id);

    for (signed k = (signed short) (left_right_bracket_stack.size - 1); k > (-1 + (signed) inverse_nested); k--) {
        symbol_id = popLeftRightBracketStack();
        symbol = getComplexElementBySymbolId(symbol, symbol_id);
    }
    return symbol;
}

void removeChildrenOfComplex(Symbol* symbol) {
    if (symbol->type == K_ARRAY || symbol->type == K_DICT) {
        for (unsigned long i = 0; i < symbol->children_count; i++) {
            removeSymbol(symbol->children[i]);
        }
    }
}

bool isComplex(Symbol* symbol) {
    return symbol->type == K_ARRAY || symbol->type == K_DICT;
}

void buildVariableComplexElement(char *name, char *key) {
    variable_complex_element = getComplexElementThroughLeftRightBracketStack(name, 1);
    variable_complex_element_symbol_id = popLeftRightBracketStack();

    if (isComplexMode()) {
        Symbol* symbol = variable_complex_element;
        symbol = getComplexElementBySymbolId(symbol, variable_complex_element_symbol_id);

        Symbol* clone_symbol = createCloneFromSymbol(
            NULL,
            symbol->type,
            symbol,
            symbol->secondary_type
        );
        clone_symbol->key = key;
    }
}

void pushNestedComplexModeStack(Symbol* complex_mode) {
    if (complex_mode_stack.arr[complex_mode_stack.size - 2]->type != K_DICT)
        return;
    if (nested_complex_mode_stack.capacity == 0) {
        nested_complex_mode_stack.arr = (Symbol**)malloc((nested_complex_mode_stack.capacity = 2) * sizeof(Symbol));
    } else if (nested_complex_mode_stack.capacity == nested_complex_mode_stack.size) {
        nested_complex_mode_stack.arr = (Symbol**)realloc(nested_complex_mode_stack.arr, (nested_complex_mode_stack.capacity *= 2) * sizeof(Symbol));
    }

    nested_complex_mode_stack.arr[nested_complex_mode_stack.size] = complex_mode;
    nested_complex_mode_stack.size++;
}

void popNestedComplexModeStack(char *key) {
    nested_complex_mode_stack.arr[nested_complex_mode_stack.size - 1]->key = key;
    nested_complex_mode_stack.arr[nested_complex_mode_stack.size - 1] = NULL;
    nested_complex_mode_stack.size--;
}

void freeNestedComplexModeStack() {
    for (unsigned i = 0; i < nested_complex_mode_stack.size; i++) {
        if (nested_complex_mode_stack.arr[i] != NULL)
            removeSymbol(nested_complex_mode_stack.arr[i]);
    }

    if (nested_complex_mode_stack.capacity > 0) {
        free(nested_complex_mode_stack.arr);
    }

    nested_complex_mode_stack.capacity = 0;
    nested_complex_mode_stack.size = 0;
}
