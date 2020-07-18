#include "extension.h"

void initKaosApi() {
    kaos.defineFunction = defineFunction;
    kaos.getVariableBool = getVariableBool;
    kaos.getVariableInt = getVariableInt;
    kaos.getVariableFloat = getVariableFloat;
    kaos.getVariableString = getVariableString;
    kaos.getVariableBoolByTypeCasting = getVariableBoolByTypeCasting;
    kaos.getVariableIntByTypeCasting = getVariableIntByTypeCasting;
    kaos.getVariableFloatByTypeCasting = getVariableFloatByTypeCasting;
    kaos.getVariableStringByTypeCasting = getVariableStringByTypeCasting;
    kaos.getListLength = getListLength;
    kaos.getListElementBool = getListElementBool;
    kaos.getListElementInt = getListElementInt;
    kaos.getListElementFloat = getListElementFloat;
    kaos.getListElementString = getListElementString;
    kaos.getListElementBoolByTypeCasting = getListElementBoolByTypeCasting;
    kaos.getListElementIntByTypeCasting = getListElementIntByTypeCasting;
    kaos.getListElementFloatByTypeCasting = getListElementFloatByTypeCasting;
    kaos.getListElementStringByTypeCasting = getListElementStringByTypeCasting;
    kaos.copyListElement = copyListElement;
    kaos.getListElementType = getListElementType;
    kaos.getListElementValueType = getListElementValueType;
    kaos.getDictLength = getDictLength;
    kaos.getDictKeyByIndex = getDictKeyByIndex;
    kaos.getDictElementBool = getDictElementBool;
    kaos.getDictElementInt = getDictElementInt;
    kaos.getDictElementFloat = getDictElementFloat;
    kaos.getDictElementString = getDictElementString;
    kaos.getDictElementBoolByTypeCasting = getDictElementBoolByTypeCasting;
    kaos.getDictElementIntByTypeCasting = getDictElementIntByTypeCasting;
    kaos.getDictElementFloatByTypeCasting = getDictElementFloatByTypeCasting;
    kaos.getDictElementStringByTypeCasting = getDictElementStringByTypeCasting;
    kaos.copyDictElement = copyDictElement;
    kaos.getDictElementType = getDictElementType;
    kaos.getDictElementValueType = getDictElementValueType;
    kaos.dumpVariableToString = dumpVariableToString;
    kaos.returnVariableBool = returnVariableBool;
    kaos.returnVariableInt = returnVariableInt;
    kaos.returnVariableFloat = returnVariableFloat;
    kaos.returnVariableString = returnVariableString;
    kaos.createVariableBool = createVariableBool;
    kaos.createVariableInt = createVariableInt;
    kaos.createVariableFloat = createVariableFloat;
    kaos.createVariableString = createVariableString;
    kaos.startBuildingList = startBuildingList;
    kaos.returnList = returnList;
    kaos.startBuildingDict = startBuildingDict;
    kaos.returnDict = returnDict;
    kaos.returnComplex = returnComplex;
    kaos.finishList = finishList;
    kaos.finishDict = finishDict;
    kaos.finishComplex = finishComplex;
    kaos.getListType = getListType;
    kaos.getDictType = getDictType;
    kaos.getValueType = getValueType;
    kaos.getRole = getRole;
    kaos.raiseError = raiseError;
    kaos.parseJson = parseJson;
}

void callRegisterInDynamicLibrary(char* dynamic_library_path) {
    dynamic_library dylib = getFunctionFromDynamicLibrary(dynamic_library_path, __KAOS_EXTENSION_REGISTER_FUNCTION__);
    dylib.func(kaos);
    if (is_interactive) {
        phase = PROGRAM;
    }
}

void callFunctionFromDynamicLibrary(_Function* function) {
    char* function_name = "";
    function_name = strcat_ext(function_name, __KAOS_EXTENSION_FUNCTION_PREFIX__);
    function_name = strcat_ext(function_name, function->name);
    dynamic_library dylib = getFunctionFromDynamicLibrary(function->module_context, function_name);
    dylib.func();
    free(function_name);
}

dynamic_library getFunctionFromDynamicLibrary(char* dynamic_library_path, char* function_name) {
    dynamic_library dylib;

    dylib.handle = OPENLIB(dynamic_library_path);

    if (dylib.handle == NULL) {
        #if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
        fprintf(stderr, "Unable to open lib: %s\n", dlerror());
        #endif
    }
    dylib.func = LIBFUNC(dylib.handle, function_name);

    if (dylib.func == NULL) {
        fprintf(stderr, "Unable to get symbol\n");
    }

    return dylib;
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
