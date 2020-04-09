#include "extension.h"

void callRegisterInDynamicLibrary(char* dynamic_library_path) {
    kaos.defineFunction = defineFunction;
    kaos.getVariableBool = getVariableBool;
    kaos.getVariableInt = getVariableInt;
    kaos.getVariableFloat = getVariableFloat;
    kaos.getVariableString = getVariableString;
    kaos.getArrayElementBool = getArrayElementBool;
    kaos.getArrayElementInt = getArrayElementInt;
    kaos.getArrayElementFloat = getArrayElementFloat;
    kaos.getArrayElementString = getArrayElementString;
    kaos.getDictElementBool = getDictElementBool;
    kaos.getDictElementInt = getDictElementInt;
    kaos.getDictElementFloat = getDictElementFloat;
    kaos.getDictElementString = getDictElementString;
    kaos.returnVariableBool = returnVariableBool;
    kaos.returnVariableInt = returnVariableInt;
    kaos.returnVariableFloat = returnVariableFloat;
    kaos.returnVariableString = returnVariableString;
    kaos.createVariableBool = createVariableBool;
    kaos.createVariableInt = createVariableInt;
    kaos.createVariableFloat = createVariableFloat;
    kaos.createVariableString = createVariableString;
    kaos.startBuildingArray = startBuildingArray;
    kaos.returnArray = returnArray;
    kaos.startBuildingDict = startBuildingDict;
    kaos.returnDict = returnDict;
    kaos.returnComplex = returnComplex;
    lib_func func = getFunctionFromDynamicLibrary(dynamic_library_path, __KAOS_EXTENSION_REGISTER_FUNCTION__);
    func(kaos);
    if (is_interactive) {
        phase = PROGRAM;
    }
}

void callFunctionFromDynamicLibrary(_Function* function) {
    char* function_name = "";
    function_name = strcat_ext(function_name, __KAOS_EXTENSION_FUNCTION_PREFIX__);
    function_name = strcat_ext(function_name, function->name);
    lib_func func = getFunctionFromDynamicLibrary(function->module_context, function_name);
    func();
    free(function_name);
}

lib_func getFunctionFromDynamicLibrary(char* dynamic_library_path, char* function_name) {
    void     *handle  = NULL;
    lib_func  func    = NULL;

    handle = OPENLIB(dynamic_library_path);

    if (handle == NULL) {
        #if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
        fprintf(stderr, "Unable to open lib: %s\n", dlerror());
        #endif
    }
    func = LIBFUNC(handle, function_name);

    if (func == NULL) {
        fprintf(stderr, "Unable to get symbol\n");
    }

    return func;
}

void returnVariable(Symbol* symbol) {
    scope_override = executed_function->parent_scope;
    executed_function->symbol = createCloneFromSymbol(
        NULL,
        symbol->type,
        symbol,
        symbol->secondary_type
    );
    decision_symbol_chain = createCloneFromSymbol(
        NULL,
        symbol->type,
        symbol,
        symbol->secondary_type
    );
    scope_override = NULL;
    removeSymbol(symbol);
}
