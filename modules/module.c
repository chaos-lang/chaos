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
    char *module_dir;

    module_dir = malloc(strlen(module_path_stack.arr[module_path_stack.size - 1]) + 1);
    strcpy(module_dir, module_path_stack.arr[module_path_stack.size - 1]);
    char *ptr = strrchr(module_dir, __PATH_SEPARATOR_ASCII__);
    if (ptr) {
        *ptr = '\0';
    } else {
        module_dir = (char *) realloc(module_dir, strlen("") + 1);
        strcpy(module_dir, "");
    }

    module_path = strcat_ext(module_path, module_dir);
    if (module_path[0] != '\0') module_path = strcat_ext(module_path, __PATH_SEPARATOR__);

    for (int i = 0; i < modules_buffer.size; i++) {
        module_path = strcat_ext(module_path, modules_buffer.arr[i]);
        if (i + 1 != modules_buffer.size) {
            module_dir = (char *) realloc(module_dir, strlen(module_path) + 1);
            strcpy(module_dir, module_path);
            module_path = strcat_ext(module_path, __PATH_SEPARATOR__);
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
    module_path = strcat_ext(module_path, __LANGUAGE_FILE_EXTENSION__);

#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
    relative_path_to_absolute(module_path);
#else
    char actual_path[PATH_MAX];
    _fullpath(actual_path, module_path, PATH_MAX);
    module_path = (char *) realloc(module_path, strlen(actual_path) + 1);
    strcpy(module_path, actual_path);
#endif
    pushModuleStack(module_path, module);

    freeModulesBuffer();

    parseTheModuleContent(module_path);

    freeFunctionNamesBuffer();

    popModuleStack();

    free(module_path);
    free(module_dir);
    free(module_name);
}

void freeModulesBuffer() {
    for (int i = 0; i < modules_buffer.size; i++) {
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
    for (int i = 0; i < module_path_stack.size; i++) {
        free(module_path_stack.arr[i]);
    }
    module_path_stack.capacity = 0;
    module_path_stack.size = 0;
    free(module_path_stack.arr);
}

void freeModuleStack() {
    for (int i = 0; i < module_stack.size; i++) {
        free(module_stack.arr[i]);
    }
    module_stack.capacity = 0;
    module_stack.size = 0;
    free(module_stack.arr);
}

char* getCurrentModule() {
    return module_path_stack.arr[module_path_stack.size - 1];
}
