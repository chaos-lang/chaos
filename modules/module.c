#include "module.h"

void initMainContext() {
    modules_buffer.capacity = 0;
    modules_buffer.size = 0;
    module_path_stack.capacity = 0;
    module_path_stack.size = 0;
    module_stack.capacity = 0;
    module_stack.size = 0;

    char *module_path_with_extension = malloc(1 + strlen(program_file_path));
    strcpy(module_path_with_extension, program_file_path);
    pushModuleStack(module_path_with_extension, "");
    free(module_path_with_extension);
}

void appendModuleToModuleBuffer(char *name) {
    append_to_array(&modules_buffer, name);
    free(name);
}

void prependModuleToModuleBuffer(char *name) {
    prepend_to_array(&modules_buffer, name);
}

void handleModuleImport(char *module_name, bool directly_import) {
    char *module_path = "";
    char *relative_path = "";
    char *module_dir;

    module_dir = malloc(strlen(module_path_stack.arr[module_path_stack.size - 1]) + 1);
    strcpy(module_dir, module_path_stack.arr[module_path_stack.size - 1]);
    char *ptr = strrchr(module_dir, __KAOS_PATH_SEPARATOR_ASCII__);
    if (ptr) {
        *ptr = '\0';
    } else {
        module_dir = (char *) realloc(module_dir, strlen("") + 1);
        strcpy(module_dir, "");
    }

    module_path = strcat_ext(module_path, module_dir);
    if (module_path[0] != '\0') module_path = strcat_ext(module_path, __KAOS_PATH_SEPARATOR__);

    for (unsigned i = 0; i < modules_buffer.size; i++) {
        module_path = strcat_ext(module_path, modules_buffer.arr[i]);
        relative_path = strcat_ext(relative_path, modules_buffer.arr[i]);
        if (i + 1 != modules_buffer.size) {
            module_path = strcat_ext(module_path, __KAOS_PATH_SEPARATOR__);
            relative_path = strcat_ext(relative_path, __KAOS_PATH_SEPARATOR__);
        }
    }

    char *module = modules_buffer.arr[modules_buffer.size - 1];
    if (module_name != NULL) {
        module = module_name;
    }
    if (directly_import) {
        module = "";
    }

    module_path = strcat_ext(module_path, ".");
    module_path = strcat_ext(module_path, __KAOS_LANGUAGE_FILE_EXTENSION__);
    relative_path = strcat_ext(relative_path, ".");
    relative_path = strcat_ext(relative_path, __KAOS_LANGUAGE_FILE_EXTENSION__);

    module_path = searchSpellsIfNotExits(module_path, relative_path);

    pushModuleStack(module_path, module);

    freeModulesBuffer();

    if (strcmp(
        get_filename_ext(module_path),
        __KAOS_DYNAMIC_LIBRARY_EXTENSION__
        ) == 0
    ) {
        callRegisterInDynamicLibrary(module_path);
    } else {
        parseTheModuleContent(module_path);
    }

    freeFunctionNamesBuffer();

    popModuleStack();

    free(module_path);
    free(relative_path);
    free(module_dir);
    free(module_name);
}

void freeModulesBuffer() {
    for (unsigned i = 0; i < modules_buffer.size; i++) {
        free(modules_buffer.arr[i]);
    }
    if (modules_buffer.size > 0) free(modules_buffer.arr);
    modules_buffer.capacity = 0;
    modules_buffer.size = 0;
}

void pushModuleStack(char *module_path, char *module) {
    append_to_array(&module_path_stack, module_path);
    append_to_array(&module_stack, module);
}

void popModuleStack() {
    free(module_path_stack.arr[module_path_stack.size - 1]);
    module_path_stack.size--;
    free(module_stack.arr[module_stack.size - 1]);
    module_stack.size--;
}

void freeModulePathStack() {
    for (unsigned i = 0; i < module_path_stack.size; i++) {
        free(module_path_stack.arr[i]);
    }
    module_path_stack.capacity = 0;
    module_path_stack.size = 0;
    free(module_path_stack.arr);
}

void freeModuleStack() {
    for (unsigned i = 0; i < module_stack.size; i++) {
        free(module_stack.arr[i]);
    }
    module_stack.capacity = 0;
    module_stack.size = 0;
    free(module_stack.arr);
}

char* getCurrentModule() {
    return module_path_stack.arr[module_path_stack.size - 1];
}

char* getMainModuleDir() {
    char *module_dir;

    module_dir = malloc(strlen(module_path_stack.arr[0]) + 1);
    strcpy(module_dir, module_path_stack.arr[0]);
    char *ptr = strrchr(module_dir, __KAOS_PATH_SEPARATOR_ASCII__);
    if (ptr) {
        *ptr = '\0';
    } else {
        module_dir = (char *) realloc(module_dir, strlen("") + 1);
        strcpy(module_dir, "");
    }

    return module_dir;
}

char* searchSpellsIfNotExits(char* module_path, char* relative_path) {
    module_path = relative_path_to_absolute(module_path);

    if (is_file_exists(module_path)) {
        return module_path;
    } else {
        free(module_path);
        char* spells_dir = strcat_ext(getMainModuleDir(), __KAOS_PATH_SEPARATOR__);
        spells_dir = strcat_ext(spells_dir, __KAOS_SPELLS__);
        spells_dir = strcat_ext(spells_dir, __KAOS_PATH_SEPARATOR__);
        module_path = strcat_ext(spells_dir, relative_path);
        module_path = relative_path_to_absolute(module_path);
        if (is_file_exists(module_path)) {
            return module_path;
        } else {
            char* dynamic_library_path = remove_ext(module_path, '.', __KAOS_PATH_SEPARATOR_ASCII__);
            dynamic_library_path = strcat_ext(dynamic_library_path, ".");
            dynamic_library_path = strcat_ext(dynamic_library_path, __KAOS_DYNAMIC_LIBRARY_EXTENSION__);
            if (is_file_exists(dynamic_library_path)) {
                return dynamic_library_path;
            }
        }
    }
    throw_error(E_MODULE_IS_EMPTY_OR_NOT_EXISTS_ON_PATH, module_path);
    return NULL;
}
