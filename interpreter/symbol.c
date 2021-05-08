/*
 * Description: Symbol module of the Chaos Programming Language's source
 *
 * Copyright (c) 2019-2021 Chaos Language Development Authority <info@chaos-lang.org>
 *
 * License: GNU General Public License v3.0
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>
 *
 * Authors: M. Mert Yildiran <me@mertyildiran.com>
 *          Melih Sahin <melihsahin24@gmail.com>
 */

#include "symbol.h"

char *type_names[] = {
    "Void",
    "Boolean",
    "Number",
    "String",
    "Any",
    "List",
    "Dictionary"
};

char *value_type_names[] = {
    "Boolean",
    "Integer",
    "Float",
    "String",
    "Any",
    "Void",
    "List",
    "Dict",
    "Ref"
};

unsigned long long symbol_id_counter = 0;
bool is_complex_parsing = false;
bool disable_complex_mode = false;

Symbol* addSymbol(char *name, enum Type type, union Value value, enum ValueType value_type) {
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
    symbol->role = DEFAULT;

    updateSymbolScope(symbol);

    addSymbolToComplex(symbol);
#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__) && !defined(CHAOS_COMPILER)
    add_suggestion(name);
#endif

    return symbol;
}

void updateSymbolScope(Symbol* symbol) {
    symbol->scope = isComplexMode() ? scopeless : getCurrentScope();

    if (symbol->scope->start_symbol == NULL) {
        symbol->scope->start_symbol = symbol;
        symbol->scope->end_symbol = symbol;
    } else {
        symbol->scope->end_symbol->next = symbol;
        symbol->previous = symbol->scope->end_symbol;
        symbol->scope->end_symbol = symbol;
        symbol->scope->end_symbol->next = NULL;
    }
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
        symbol->scope->start_symbol = NULL;
        symbol->scope->end_symbol = NULL;
        freeSymbol(symbol);
        return;
    } else if (previous_symbol == NULL) {
        symbol->scope->start_symbol = next_symbol;
        symbol->scope->start_symbol->previous = NULL;
    } else if (next_symbol == NULL) {
        symbol->scope->end_symbol = previous_symbol;
        symbol->scope->end_symbol->next = NULL;
    } else {
        previous_symbol->next = next_symbol;
        next_symbol->previous = previous_symbol;
    }

    freeSymbol(symbol);
}

void freeSymbol(Symbol* symbol) {
    // if (symbol->value_type == V_STRING) free(symbol->value.s);
    if (symbol->children_count > 0) free(symbol->children);
    free(symbol->key);
    free(symbol->name);
    free(symbol->secondary_name);
    free(symbol);
}

Symbol* findSymbol(char *name) {
    FunctionCall* scope = getCurrentScope();
    symbol_cursor = scope->start_symbol;
    while (symbol_cursor != NULL) {
        if (
            symbol_cursor->name != NULL &&
            strcmp(symbol_cursor->name, name) == 0
        ) {
            Symbol* symbol = symbol_cursor;
            return symbol;
        }
        symbol_cursor = symbol_cursor->next;
    }
    return NULL;
}

Symbol* getSymbol(char *name) {
    Symbol* symbol = findSymbol(name);
    if (symbol != NULL)
        return symbol;
    throw_error(E_UNDEFINED_VARIABLE, name);
    return NULL;
}

Symbol* getSymbolById(unsigned long long id) {
    FunctionCall* scope = getCurrentScope();
    symbol_cursor = scope->start_symbol;
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
    if (type == K_LIST || type == K_DICT) {
        clone_symbol = deepCopyComplex(NULL, symbol);
        clone_symbol->key = NULL;
        if (key != NULL) {
            clone_symbol->key = malloc(1 + strlen(key));
            strcpy(clone_symbol->key, key);
        }
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
    if (symbol->type == K_LIST) {
        addSymbolList(NULL);
    } else if (symbol->type == K_DICT) {
        addSymbolDict(NULL);
    } else {
        append_to_array_without_malloc(&free_string_stack, name);
        throw_error(E_UNRECOGNIZED_COMPLEX_DATA_TYPE, getTypeName(symbol->type), name);
    }

    for (unsigned long i = 0; i < symbol->children_count; i++) {
        Symbol* child = symbol->children[i];
        deepCopySymbol(child, child->type, child->key);
    }

    Symbol* symbol_return = complex_mode_stack.arr[complex_mode_stack.size - 1];
    finishComplexMode(name, symbol->secondary_type);

    return symbol_return;
}

char* getSymbolValueString(char *name) {
    Symbol* symbol = getSymbol(name);
    return _getSymbolValueString(symbol);
}

char* _getSymbolValueString(Symbol* symbol) {
    char* value;
    if (symbol->value_type == V_STRING) {
        value = malloc(1 + strlen(symbol->value.s));
        strcpy(value, symbol->value.s);
        return value;
    } else {
        throw_error(E_UNEXPECTED_VALUE_TYPE, symbol->name, NULL, 0, symbol->value_type);
    }
    return "";
}

double getSymbolValueFloat(char *name) {
    Symbol* symbol = getSymbol(name);
    return _getSymbolValueFloat(symbol);
}

double _getSymbolValueFloat(Symbol* symbol) {
    double value;
    switch (symbol->value_type) {
    case V_BOOL:
        value = symbol->value.b ? 1.0 : 0.0;
        return value;
        break;
    case V_INT:
        value = (double)symbol->value.i;
        return value;
        break;
    case V_FLOAT:
        value = symbol->value.f;
        return value;
        break;
    default:
        throw_error(E_UNEXPECTED_VALUE_TYPE, symbol->name, NULL, 0, symbol->value_type);
        break;
    }
    return 0.0;
}

bool getSymbolValueBool(char *name) {
    Symbol* symbol = getSymbol(name);
    return _getSymbolValueBool(symbol);
}

bool _getSymbolValueBool(Symbol* symbol) {
    bool value;
    switch (symbol->value_type) {
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
    return _getSymbolValueInt(symbol);
}

long long _getSymbolValueInt(Symbol* symbol) {
    long long value;
    switch (symbol->value_type) {
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
        throw_error(E_UNEXPECTED_VALUE_TYPE, symbol->name, NULL, 0, symbol->value_type);
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
    long long value = 0;
    switch (symbol->value_type) {
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
        throw_error(E_UNEXPECTED_VALUE_TYPE, symbol->name, NULL, 0, symbol->value_type);
        break;
    }
    return value * symbol->sign;
}

void printSymbolValue(Symbol* symbol, bool is_complex, bool pretty, bool escaped, unsigned long iter) {
    char *encoded = NULL;
    encoded = encodeSymbolValueToString(symbol, is_complex, pretty, escaped, iter, encoded, false);
    printf("%s", encoded);
    free(encoded);
}

char* encodeSymbolValueToString(Symbol* symbol, bool is_complex, bool pretty, bool escaped, unsigned long iter, char *encoded, bool double_quotes) {
    if (symbol->value_type == V_REF) {
        encoded = strcat_ext(encoded, "(ref)");
        return encoded;
    }
    switch (symbol->type) {
    case K_BOOL:
        switch (symbol->value_type) {
        case V_BOOL:
            encoded = strcat_ext(encoded, symbol->value.b ? "true" : "false");
            break;
        case V_VOID:
            encoded = strcat_ext(encoded, "N/A");
            break;
        default:
            throw_error(E_UNEXPECTED_VALUE_TYPE, symbol->name, NULL, 0, symbol->value_type);
            break;
        }
        return encoded;
    case K_NUMBER:
        switch (symbol->value_type) {
        case V_INT:
            encoded = snprintf_concat_int(encoded, "%lld", symbol->value.i);
            break;
        case V_FLOAT:
            encoded = snprintf_concat_float(encoded, "%Lg", symbol->value.f);
            break;
        case V_VOID:
            encoded = strcat_ext(encoded, "N/A");
            break;
        default:
            throw_error(E_UNEXPECTED_VALUE_TYPE, symbol->name, NULL, 0, symbol->value_type);
            break;
        }
        return encoded;
    case K_STRING:
        if (symbol->value_type == V_VOID || symbol->value.s == NULL) {
            return strcat_ext(encoded, "N/A");
        }
        if (is_complex) {
            if (double_quotes) {
                encoded = snprintf_concat_string(encoded, "\"%s\"", symbol->value.s);
            } else {
                encoded = snprintf_concat_string(encoded, "'%s'", symbol->value.s);
            }
        } else {
            if (escaped) {
                char* out = escape_the_sequences_in_string_literal(symbol->value.s);
                encoded = strcat_ext(encoded, out);
                free(out);
            } else {
                encoded = strcat_ext(encoded, symbol->value.s);
            }
        }
        return encoded;
    case K_LIST:
        iter++;
        encoded = strcat_ext(encoded, "[");
        if (pretty) {
            encoded = strcat_ext(encoded, "\n");
        }
        for (unsigned long i = 0; i < symbol->children_count; i++) {
            if (pretty) {
                for (unsigned long j = 0; j < iter; j++) {
                    encoded = strcat_ext(encoded, __KAOS_TAB__);
                }
            }
            encoded = encodeSymbolValueToString(symbol->children[i], true, pretty, escaped, iter, encoded, double_quotes);
            if (i + 1 != symbol->children_count) {
                if (pretty) {
                    encoded = strcat_ext(encoded, ",\n");
                } else {
                    encoded = strcat_ext(encoded, ", ");
                }
            }
        }
        if (pretty) {
            encoded = strcat_ext(encoded, "\n");
        }
        if (pretty) {
            for (unsigned long j = 0; j < (iter - 1); j++) {
                encoded = strcat_ext(encoded, __KAOS_TAB__);
            }
        }
        encoded = strcat_ext(encoded, "]");
        return encoded;
    case K_DICT:
        iter++;
        encoded = strcat_ext(encoded, "{");
        if (pretty) {
            encoded = strcat_ext(encoded, "\n");
        }
        for (unsigned long i = 0; i < symbol->children_count; i++) {
            if (pretty) {
                for (unsigned long j = 0; j < iter; j++) {
                    encoded = strcat_ext(encoded, __KAOS_TAB__);
                }
            }
            Symbol* child = symbol->children[i];
            if (double_quotes) {
                encoded = snprintf_concat_string(encoded, "\"%s\": ", child->key);
            } else {
                encoded = snprintf_concat_string(encoded, "'%s': ", child->key);
            }
            encoded = encodeSymbolValueToString(child, true, pretty, escaped, iter, encoded, double_quotes);
            if (i + 1 != symbol->children_count) {
                if (pretty) {
                    encoded = strcat_ext(encoded, ",\n");
                } else {
                    encoded = strcat_ext(encoded, ", ");
                }
            }
        }
        if (pretty) {
            encoded = strcat_ext(encoded, "\n");
        }
        if (pretty) {
            for (unsigned long j = 0; j < (iter - 1); j++) {
                encoded = strcat_ext(encoded, __KAOS_TAB__);
            }
        }
        encoded = strcat_ext(encoded, "}");
        return encoded;
    case K_ANY:
        switch (symbol->value_type) {
        case V_STRING:
            encoded = strcat_ext(encoded, symbol->value.s);
            break;
        case V_INT:
            encoded = snprintf_concat_int(encoded, "%lld", symbol->value.i);
            break;
        case V_FLOAT:
            encoded = snprintf_concat_float(encoded, "%Lg", symbol->value.f);
            break;
        case V_BOOL:
            encoded = strcat_ext(encoded, symbol->value.b ? "true" : "false");
            break;
        case V_VOID:
            encoded = strcat_ext(encoded, "N/A");
            break;
        default:
            throw_error(E_UNEXPECTED_VALUE_TYPE, symbol->name, NULL, 0, symbol->value_type);
            break;
        }
        return encoded;
    default:
        throw_error(E_UNKNOWN_VARIABLE_TYPE, getTypeName(symbol->type), symbol->name);
        break;
    }
    return encoded;
}

void printSymbolValueEndWith(Symbol* symbol, char *end, bool pretty, bool escaped) {
    printSymbolValue(symbol, false, pretty, escaped, 0);
    printf("%s", end);
}

void printSymbolValueEndWithNewLine(Symbol* symbol, bool pretty, bool escaped) {
    printSymbolValueEndWith(symbol, "\n", pretty, escaped);
}

bool isDefined(char *name) {
    FunctionCall* scope = getCurrentScope();
    symbol_cursor = scope->start_symbol;
    while (symbol_cursor != NULL) {
        if (
            symbol_cursor->name != NULL &&
            name != NULL && strcmp(symbol_cursor->name, name) == 0
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
        throw_error(E_MEMORY_ALLOCATION_FOR_LIST_FAILED, complex_mode_stack.arr[complex_mode_stack.size - 1]->name);
    }

    complex_mode_stack.arr[complex_mode_stack.size - 1]->children[
        complex_mode_stack.child_counter[complex_mode_stack.size - 1] - 1
    ] = symbol;
}

void printSymbolTable() {
    for (unsigned i = 0; i < function_call_stack.size + 1; i++) {
        Symbol *symbol;
        if (i == 0) {
            symbol = scopeless->start_symbol;
            printf("[scope]: %s %p\n", scopeless->function->name, (void *)scopeless->function);
        } else {
            symbol = function_call_stack.arr[i - 1]->start_symbol;
            printf("[scope]: %s %p\n", function_call_stack.arr[i - 1]->function->name, (void *)function_call_stack.arr[i - 1]->function);
        }

        printf("\t[start] =>\n");
        while(symbol != NULL) {
            char *encoded = NULL;
            encoded = encodeSymbolValueToString(symbol, false, false, true, 0, encoded, false);
            FunctionCall* scope = symbol->scope;
            printf(
                "\t\t{id: %llu, name: %s, 2nd_name: %s, key: %s, scope: %s, type: %u, 2nd_type: %u, value_type: %u, role: %u, param_of: %s, value: %s} =>\n",
                symbol->id,
                symbol->name,
                symbol->secondary_name,
                symbol->key,
                scope->function->name,
                symbol->type,
                symbol->secondary_type,
                symbol->value_type,
                symbol->role,
                symbol->param_of != NULL ? symbol->param_of->name : "",
                encoded
            );
            symbol = symbol->next;
            free(encoded);
        }
        printf("\t[end]\n");
    }
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

Symbol* addSymbolFloat(char *name, double f) {
    union Value value;
    value.f = f;
    return addSymbol(name, K_NUMBER, value, V_FLOAT);
}

void updateSymbolFloat(char *name, double f) {
    union Value value;
    value.f = f;
    updateSymbol(name, K_NUMBER, value, V_FLOAT);
}

Symbol* addSymbolString(char *name, char *s) {
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    return addSymbol(name, K_STRING, value, V_STRING);
}

void updateSymbolString(char *name, char *s) {
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    updateSymbol(name, K_STRING, value, V_STRING);
}

void addSymbolList(char *name) {
    union Value value;
    value.i = 0;
    Symbol* complex_mode = addSymbol(name, K_LIST, value, V_VOID);
    pushComplexModeStack(complex_mode);
}

Symbol* createCloneFromSymbolByName(char *clone_name, enum Type type, char *name, enum Type extra_type) {
    Symbol* symbol = getSymbol(name);
    Symbol* clone_symbol = createCloneFromSymbol(clone_name, type, symbol, extra_type);
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
        throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, getTypeName(type), clone_name);
    }

    if (clone_name != NULL && findSymbol(clone_name) != NULL) {
        throw_error(E_VARIABLE_ALREADY_DEFINED, clone_name);
    }

    Symbol* clone_symbol;
    if (symbol->type == K_LIST || symbol->type == K_DICT) {
        if (symbol->secondary_type != extra_type) {
            throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, getTypeName(extra_type), clone_name);
        }
        clone_symbol = deepCopyComplex(clone_name, symbol);
    } else {
        if (type == K_ANY) {
            clone_symbol = deepCopySymbol(symbol, K_ANY, NULL);
        } else if (symbol->type == K_ANY) {
            Symbol* temp_symbol = createSymbolWithoutValueType(clone_name, type);
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
            throw_error(E_VARIABLE_ALREADY_DEFINED, name);
        }

        complex_mode->name = malloc(1 + strlen(name));
        strcpy(complex_mode->name, name);
    }
    complex_mode->secondary_type = type;
    enum Type illegal_type = isComplexIllegal(type);
    if (illegal_type != (enum Type)-1) {
        if (name != NULL)
            free(name);
        throw_error(E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_LIST, getTypeName(illegal_type), complex_mode->name);
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

Symbol* getListElement(Symbol* symbol, long long i) {
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

    if (symbol->type != K_LIST) throw_error(E_VARIABLE_IS_NOT_AN_LIST, symbol->name);

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
    deepCopySymbol(symbol, symbol->type, key);
}

Symbol* getComplexElement(Symbol* complex, long long i, char *key) {

    Symbol* symbol = NULL;
    if (complex->type == K_LIST || complex->type == K_STRING) {
        symbol = getListElement(complex, i);
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

    Symbol* symbol = NULL;
    if (complex->type == K_LIST || complex->type == K_STRING) {
        symbol = getListElement(complex, i);
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
            throw_error(E_INDEX_OUT_OF_RANGE_STRING, complex->name, NULL, orig_i);
        }

        complex->value.s[i] = value.s[0];
        free(value.s);
        return;
    } else if (complex->secondary_type != K_ANY && complex->secondary_type != type) {
        free(key);
        if (type == K_STRING)
            free(value.s);
        throw_error(E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_LIST, getTypeName(type), complex->name);
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

void updateComplexElementFloat(double f) {
    union Value value;
    value.f = f;
    updateComplexElementWrapper(K_NUMBER, value, V_FLOAT);
}

void updateComplexElementString(char *s) {
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    updateComplexElementWrapper(K_STRING, value, V_STRING);
}

void updateComplexElementSymbol(Symbol* source) {
    Symbol* complex = variable_complex_element;
    unsigned long long symbol_id = variable_complex_element_symbol_id;

    _updateComplexElementSymbol(complex, symbol_id, source);
}

void _updateComplexElementSymbol(Symbol* complex, unsigned long long symbol_id, Symbol* source) {
    Symbol* access_symbol = getSymbolById(symbol_id);

    if (access_symbol->type == K_LIST || access_symbol->type == K_DICT) {
        throw_error(E_UNEXPECTED_ACCESSOR_DATA_TYPE, getTypeName(access_symbol->type), complex->name);
    }

    long long i = getSymbolValueInt_ZeroIfNotInt(access_symbol);
    char *key = getSymbolValueString_NullIfNotString(access_symbol);

    if (complex->type != K_LIST && complex->type != K_DICT) {
        removeSymbol(access_symbol);
        free(key);
        throw_error(E_UNRECOGNIZED_COMPLEX_DATA_TYPE, getTypeName(complex->type), complex->name);
    }

    if (complex->secondary_type != K_ANY && complex->secondary_type != source->type) {
        removeSymbol(access_symbol);
        free(key);
        throw_error(E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_LIST, getTypeName(source->type), complex->name);
    }

    Symbol* symbol;
    if (complex->type == K_LIST) {
        removeSymbol(access_symbol);
        symbol = getListElement(complex, i);
        removeSymbol(symbol);
        complex->children[i] = deepCopySymbol(source, source->type, NULL);
    } else if (complex->type == K_DICT) {
        removeComplexElement(complex, symbol_id);
        pushComplexModeStack(complex);
        complex_mode_stack.child_counter[complex_mode_stack.size - 1] = complex->children_count;
        deepCopySymbol(source, source->type, key);
        finishComplexMode(NULL, complex->secondary_type);
    }
    free(key);
}

void removeComplexElement(Symbol* complex, unsigned long long symbol_id) {
    Symbol* access_symbol = getSymbolById(symbol_id);
    long long i = getSymbolValueInt_ZeroIfNotInt(access_symbol);
    char *key = getSymbolValueString_NullIfNotString(access_symbol);
    removeSymbol(access_symbol);

    Symbol* symbol = NULL;
    if (complex->type == K_STRING) {
        long long orig_i = i;

        if (i < 0) {
            i = strlen(complex->value.s) + i;
        }

        if (i < 0 || i > strlen(complex->value.s) - 1) {
            throw_error(E_INDEX_OUT_OF_RANGE_STRING, complex->name, NULL, orig_i);
        }

        remove_nth_char(complex->value.s, i);
        return;
    } else if (complex->type == K_LIST) {
        symbol = getListElement(complex, i);
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
    value.i = 0;
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

Symbol* addSymbolAnyStringNew(char *name, char *s, size_t len) {
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    Symbol* symbol = addSymbol(name, K_ANY, value, V_STRING);
    symbol->len = len;
    return symbol;
}

Symbol* addSymbolAnyString(char *name, char *s) {
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    return addSymbol(name, K_ANY, value, V_STRING);
}

Symbol* addSymbolAnyInt(char *name, long long i) {
    union Value value;
    value.i = i;
    return addSymbol(name, K_ANY, value, V_INT);
}

Symbol* addSymbolAnyFloat(char *name, double f) {
    union Value value;
    value.f = f;
    return addSymbol(name, K_ANY, value, V_FLOAT);
}

Symbol* addSymbolAnyBool(char *name, bool b) {
    union Value value;
    value.b = b;
    return addSymbol(name, K_ANY, value, V_BOOL);
}

FunctionCall* getCurrentScope() {
    if (scope_override != NULL) return scope_override;

    if (function_call_stack.size > 0) {
        return function_call_stack.arr[function_call_stack.size - 1];
    } else {
        return scopeless;
    }
}

Symbol* getSymbolFunctionParameter(char *name) {
    if (function_call_stack.size > 0) {
        scope_override = function_call_stack.arr[function_call_stack.size - 1];
    } else {
        scope_override = scopeless;
    }

    Symbol* symbol = getSymbol(name);
    scope_override = function_call_start;
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
    for (unsigned i = 0; i < function_call_stack.size + 1; i++) {
        if (i == 0)
            symbol_cursor = scopeless->start_symbol;
        else
            symbol_cursor = function_call_stack.arr[i - 1]->start_symbol;

        while (symbol_cursor != NULL) {
            Symbol* symbol = symbol_cursor;
            symbol_cursor = symbol_cursor->next;
            freeSymbol(symbol);
        }
    }
}

Symbol* assignByTypeCasting(Symbol* clone_symbol, Symbol* symbol) {
    switch (clone_symbol->value_type) {
    case V_BOOL:
        clone_symbol->value.b = symbolValueByTypeCastingToBool(symbol);
        break;
    case V_INT:
        clone_symbol->value.i = symbolValueByTypeCastingToInt(symbol);
        break;
    case V_FLOAT:
        clone_symbol->value.f = symbolValueByTypeCastingToFloat(symbol);
        break;
    case V_STRING:
        free(clone_symbol->value.s);
        clone_symbol->value.s = symbolValueByTypeCastingToString(symbol);
        break;
    default:
        throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, getTypeName(symbol->type), clone_symbol->name);
        break;
    }
    return deepCopySymbol(clone_symbol, clone_symbol->type, NULL);
}

bool symbolValueByTypeCastingToBool(Symbol* symbol) {
    switch (symbol->value_type) {
    case V_BOOL:
        return symbol->value.b;
    case V_INT:
        if (symbol->value.i != 0) {
            return true;
        } else {
            return false;
        }
    case V_FLOAT:
        if (symbol->value.f != 0.0) {
            return true;
        } else {
            return false;
        }
    case V_STRING:
        if (symbol->value.s[0] != '\0') {
            return true;
        } else {
            return false;
        }
    default:
        return false;
    }
}

long long symbolValueByTypeCastingToInt(Symbol* symbol) {
    switch (symbol->value_type) {
    case V_BOOL:
        return symbol->value.b ? 1 : 0;
    case V_INT:
        return symbol->value.i;
    case V_FLOAT:
        return (long long)symbol->value.f;
    case V_STRING:
        return atoi(symbol->value.s);
    default:
        return 0;
    }
}

double symbolValueByTypeCastingToFloat(Symbol* symbol) {
    switch (symbol->value_type) {
    case V_BOOL:
        return symbol->value.b ? 1.0 : 0.0;
    case V_INT:
        return (double) symbol->value.i;
    case V_FLOAT:
        return symbol->value.f;
    case V_STRING:
        return atof(symbol->value.s);
    default:
        return 0.0;
    }
}

char* symbolValueByTypeCastingToString(Symbol* symbol) {
    char buffer[__KAOS_ITOA_BUFFER_LENGTH__];
    char *val;
    char *result;
    switch (symbol->value_type) {
    case V_BOOL:
        val = symbol->value.b ? "true" : "false";
        result = malloc(1 + strlen(val));
        strcpy(result, val);
        return result;
    case V_INT:
        val = longlong_to_string(symbol->value.i, buffer, 10);
        result = malloc(1 + strlen(val));
        strcpy(result, val);
        return result;
    case V_FLOAT:
        sprintf(buffer, "%lg", symbol->value.f);
        val = buffer;
        result = malloc(1 + strlen(val));
        strcpy(result, val);
        return result;
    case V_STRING:
        val = symbol->value.s;
        result = malloc(1 + strlen(val));
        strcpy(result, val);
        return result;
    default:
        val = "";
        result = malloc(1 + strlen(val));
        strcpy(result, val);
        return result;
        break;
    }
}

Symbol* createSymbolWithoutValueType(char *name, enum Type type) {
    union Value value;
    enum ValueType value_type;

    switch (type) {
    case K_BOOL:
        value.b = false;
        value_type = V_BOOL;
        break;
    case K_NUMBER:
        value.f = 0.0;
        value_type = V_FLOAT;
        break;
    case K_STRING:
        value.s = malloc(1 + strlen(""));
        strcpy(value.s, "");
        value_type = V_STRING;
        break;
    default:
        value.i = 0;
        value_type = V_INT;
        append_to_array_without_malloc(&free_string_stack, name);
        throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, name);
        break;
    }

    return addSymbol(name, type, value, value_type);
}

void removeSymbolsByScope(FunctionCall* scope) {
    symbol_cursor = scope->start_symbol;
    while (symbol_cursor != NULL) {
        Symbol* symbol = symbol_cursor;
        symbol_cursor = symbol_cursor->next;
        removeSymbol(symbol);
    }
}

long long incrementThenAssign(char *name, long long i) {
    updateSymbolInt(name, getSymbolValueInt(name) + i);
    return getSymbolValueInt(name);
}

long long assignThenIncrement(char *name, long long i) {
    long long result = getSymbolValueInt(name);
    updateSymbolInt(name, getSymbolValueInt(name) + i);
    return result;
}

char* getTypeName(unsigned i) {
    return type_names[i];
}

char* getValueTypeName(unsigned i) {
    return value_type_names[i];
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

void freeLeftRightBracketStackSymbols() {
    Symbol* symbol;
    for (unsigned i = 0; i < left_right_bracket_stack.size; i++) {
        symbol = getSymbolById(left_right_bracket_stack.arr[i]);
        removeSymbol(symbol);
    }
}

Symbol* getComplexElementThroughLeftRightBracketStack(char *name, unsigned long inverse_nested) {
    Symbol* symbol = getSymbol(name);

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
    if (symbol->type == K_LIST || symbol->type == K_DICT) {
        for (unsigned long i = 0; i < symbol->children_count; i++) {
            removeSymbol(symbol->children[i]);
        }
    }
}

bool isComplex(Symbol* symbol) {
    return symbol->type == K_LIST || symbol->type == K_DICT;
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
        if (key != NULL) {
            clone_symbol->key = malloc(1 + strlen(key));
            strcpy(clone_symbol->key, key);
        }
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
    nested_complex_mode_stack.arr[nested_complex_mode_stack.size - 1]->key = malloc(1 + strlen(key));
    strcpy(nested_complex_mode_stack.arr[nested_complex_mode_stack.size - 1]->key, key);
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

void reverseComplexMode() {
    Symbol* complex_mode = getComplexMode();
    complex_mode->children_count = complex_mode_stack.child_counter[complex_mode_stack.size - 1];
    if (complex_mode->children_count == 0) return;
    unsigned long i = 0;
    unsigned long j = complex_mode->children_count - 1;
    while (i < j) {
        Symbol* left = complex_mode->children[i];
        Symbol* right = complex_mode->children[j];
        complex_mode->children[i] = right;
        complex_mode->children[j] = left;
        i++;
        j--;
    }
}

bool resolveRelEqualUnknown(char* name_l, char* name_r) {
    Symbol* symbol_l = getSymbol(name_l);
    Symbol* symbol_r = getSymbol(name_r);

    switch (symbol_l->value_type) {
    case V_BOOL:
        return symbol_l->value.b == _getSymbolValueBool(symbol_r);
        break;
    case V_INT:
        return symbol_l->value.i == _getSymbolValueInt(symbol_r);
        break;
    case V_FLOAT:
        return symbol_l->value.f == _getSymbolValueFloat(symbol_r);
        break;
    default:
        throw_error(E_UNEXPECTED_VALUE_TYPE, symbol_l->name, NULL, 0, symbol_l->value_type);
        break;
    }
    return 0;
}

bool resolveRelNotEqualUnknown(char* name_l, char* name_r) {
    Symbol* symbol_l = getSymbol(name_l);
    Symbol* symbol_r = getSymbol(name_r);

    switch (symbol_l->value_type) {
    case V_BOOL:
        return symbol_l->value.b != _getSymbolValueBool(symbol_r);
        break;
    case V_INT:
        return symbol_l->value.i != _getSymbolValueInt(symbol_r);
        break;
    case V_FLOAT:
        return symbol_l->value.f != _getSymbolValueFloat(symbol_r);
        break;
    default:
        throw_error(E_UNEXPECTED_VALUE_TYPE, symbol_l->name, NULL, 0, symbol_l->value_type);
        break;
    }
    return 0;
}

bool resolveRelGreatUnknown(char* name_l, char* name_r) {
    Symbol* symbol_l = getSymbol(name_l);
    Symbol* symbol_r = getSymbol(name_r);

    switch (symbol_l->value_type) {
    case V_BOOL:
        return symbol_l->value.b > _getSymbolValueBool(symbol_r);
        break;
    case V_INT:
        return symbol_l->value.i > _getSymbolValueInt(symbol_r);
        break;
    case V_FLOAT:
        return symbol_l->value.f > _getSymbolValueFloat(symbol_r);
        break;
    default:
        throw_error(E_UNEXPECTED_VALUE_TYPE, symbol_l->name, NULL, 0, symbol_l->value_type);
        break;
    }
    return 0;
}

bool resolveRelSmallUnknown(char* name_l, char* name_r) {
    Symbol* symbol_l = getSymbol(name_l);
    Symbol* symbol_r = getSymbol(name_r);

    switch (symbol_l->value_type) {
    case V_BOOL:
        return symbol_l->value.b < _getSymbolValueBool(symbol_r);
        break;
    case V_INT:
        return symbol_l->value.i < _getSymbolValueInt(symbol_r);
        break;
    case V_FLOAT:
        return symbol_l->value.f < _getSymbolValueFloat(symbol_r);
        break;
    default:
        throw_error(E_UNEXPECTED_VALUE_TYPE, symbol_l->name, NULL, 0, symbol_l->value_type);
        break;
    }
    return 0;
}

bool resolveRelGreatEqualUnknown(char* name_l, char* name_r) {
    Symbol* symbol_l = getSymbol(name_l);
    Symbol* symbol_r = getSymbol(name_r);

    switch (symbol_l->value_type) {
    case V_BOOL:
        return symbol_l->value.b >= _getSymbolValueBool(symbol_r);
        break;
    case V_INT:
        return symbol_l->value.i >= _getSymbolValueInt(symbol_r);
        break;
    case V_FLOAT:
        return symbol_l->value.f >= _getSymbolValueFloat(symbol_r);
        break;
    default:
        throw_error(E_UNEXPECTED_VALUE_TYPE, symbol_l->name, NULL, 0, symbol_l->value_type);
        break;
    }
    return 0;
}

bool resolveRelSmallEqualUnknown(char* name_l, char* name_r) {
    Symbol* symbol_l = getSymbol(name_l);
    Symbol* symbol_r = getSymbol(name_r);

    switch (symbol_l->value_type) {
    case V_BOOL:
        return symbol_l->value.b <= _getSymbolValueBool(symbol_r);
        break;
    case V_INT:
        return symbol_l->value.i <= _getSymbolValueInt(symbol_r);
        break;
    case V_FLOAT:
        return symbol_l->value.f <= _getSymbolValueFloat(symbol_r);
        break;
    default:
        throw_error(E_UNEXPECTED_VALUE_TYPE, symbol_l->name, NULL, 0, symbol_l->value_type);
        break;
    }
    return 0;
}

void changeSymbolScope(Symbol* symbol, FunctionCall* scope) {
    Symbol* previous_symbol = symbol->previous;
    Symbol* next_symbol = symbol->next;

    if (previous_symbol == NULL && next_symbol == NULL) {
        symbol->scope->start_symbol = NULL;
        symbol->scope->end_symbol = NULL;
    } else if (previous_symbol == NULL) {
        symbol->scope->start_symbol = next_symbol;
        symbol->scope->start_symbol->previous = NULL;
    } else if (next_symbol == NULL) {
        symbol->scope->end_symbol = previous_symbol;
        symbol->scope->end_symbol->next = NULL;
    } else {
        previous_symbol->next = next_symbol;
        next_symbol->previous = previous_symbol;
    }

    symbol->scope = scope;

    if (symbol->scope->start_symbol == NULL) {
        symbol->scope->start_symbol = symbol;
        symbol->scope->end_symbol = symbol;
    } else {
        symbol->scope->end_symbol->next = symbol;
        symbol->previous = symbol->scope->end_symbol;
        symbol->scope->end_symbol = symbol;
        symbol->scope->end_symbol->next = NULL;
    }
}
