#include "module_new.h"

File* handleModuleImport(char *module_name, bool directly_import, char *parent_context) {
    addFile();
    char *module_path = resolveModulePath(module_name, directly_import, parent_context);
    moduleImportParse(module_path);
    // moduleImportCleanUp(module_path);
    return _ast_root->files[_ast_root->file_count - 1];
}
