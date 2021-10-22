find_package(BISON)
find_package(FLEX)

BISON_TARGET(MyParser "${CMAKE_SOURCE_DIR}/parser/parser.y" "${CMAKE_SOURCE_DIR}/parser/parser.py")
FLEX_TARGET(MyScanner "lexer.l" "lexer.h")
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

install(FILES "${BISON_MyParser_OUTPUTS}" "${FLEX_MyScanner_OUTPUTS}" DESTINATION "include")
