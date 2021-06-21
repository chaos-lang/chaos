#ifndef KAOS_MODULE_NEW_H
#define KAOS_MODULE_NEW_H

#include "module.h"
#include "../ast/ast.h"

File* handleModuleImport(char *module_name, bool directly_import, char *parent_context);

#endif
