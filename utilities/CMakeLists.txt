get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

include(../cmake/modules/FindReadline.cmake)

set(Header_Files
        "cwalk.h"
        "helpers.h"
        "language.h"
        "messages.h"
        "platform.h"
        "shell.h"
)
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files
        "cwalk.c"
        "helpers.c"
        "language.c"
        "messages.c"
        "shell.c"
)
source_group("Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" "${LIBRARY_TYPE_FLAG}" "${Header_Files}" "${Source_Files}")
target_include_directories(
        "${LIBRARY_NAME}"
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:include>"
)

if (READLINE_FOUND)
    include(CheckSymbolExists)
    check_symbol_exists(rl_bind_keyseq "stdio.h;readline/readline.h" HAVE_RL_BIND_KEYSEQ)

    target_compile_definitions("${LIBRARY_NAME}" PUBLIC -DHAVE_RL_BIND_KEYSEQ="${HAVE_RL_BIND_KEYSEQ}")

    target_include_directories("${LIBRARY_NAME}" PUBLIC "${Readline_INCLUDE_DIR}")
    target_link_libraries("${LIBRARY_NAME}" PUBLIC "${Readline_LIBRARY}")
endif ()

set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

# install rules
include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_SOURCE_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

# setup the version numbering
set_property(TARGET "${LIBRARY_NAME}" PROPERTY VERSION "1.0.0")
set_property(TARGET "${LIBRARY_NAME}" PROPERTY SOVERSION "1")

set(installable_libs "${LIBRARY_NAME}" "${PROJECT_NAME}_compiler_flags")
install(FILES "${Header_Files}" DESTINATION "include")

if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif ()
install(TARGETS ${installable_libs}
        DESTINATION "lib"
        EXPORT "${LIBRARY_NAME}Targets")
