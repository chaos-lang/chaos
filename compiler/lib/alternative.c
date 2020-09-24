#include "alternative.h"

void freeEverything() {
    free(main_function);
    free(scopeless);
    freeAllSymbols();
    freeAllFunctions();
    freeModulesBuffer();
    freeFunctionNamesBuffer();
    free(function_names_buffer.arr);
    if (strlen(decision_buffer) > 0) free(decision_buffer);
    freeModulePathStack();
    freeModuleStack();
    freeComplexModeStack();
    freeLeftRightBracketStack();
    freeFreeStringStack();
    freeNestedComplexModeStack();
    free(function_call_stack.arr);
}

void breakLoop() {
}

void continueLoop() {
}
