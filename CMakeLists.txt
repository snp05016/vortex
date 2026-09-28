cmake_minimum_required(VERSION 3.15)
project(Vortex LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_library(vortex_compiler
    src/frontend/debug/ast_debug_printer.cpp
    src/frontend/lexer/lexer.cpp
    src/frontend/parser/parser.cpp
    src/frontend/parser/parser_expressions.cpp
    src/frontend/parser/parser_statements.cpp
    src/frontend/parser/parser_declarations.cpp
    src/frontend/parser/parser_types.cpp
    )
target_include_directories(vortex_compiler PUBLIC src)
target_compile_options(vortex_compiler PRIVATE -std=c++26)

add_executable(vortex_debug
    src/main.cpp
)

target_link_libraries(vortex_debug PRIVATE vortex_compiler)
target_compile_options(vortex_debug PRIVATE -std=c++26)

add_executable(vortex_lexer_tests
    tests/cpp/lexer_tests.cpp
)

target_link_libraries(vortex_lexer_tests PRIVATE vortex_compiler)
target_compile_options(vortex_lexer_tests PRIVATE -std=c++26)

add_test(NAME lexer_tests COMMAND vortex_lexer_tests)

add_executable(vortex_parser_expression_tests
    tests/cpp/parser_expression_tests.cpp
)

target_link_libraries(vortex_parser_expression_tests PRIVATE vortex_compiler)
target_compile_options(vortex_parser_expression_tests PRIVATE -std=c++26)

add_test(NAME parser_expression_tests COMMAND vortex_parser_expression_tests)

add_executable(vortex_parser_type_tests
    tests/cpp/parser_type_tests.cpp
)

target_link_libraries(vortex_parser_type_tests PRIVATE vortex_compiler)
target_compile_options(vortex_parser_type_tests PRIVATE -std=c++26)

add_test(NAME parser_type_tests COMMAND vortex_parser_type_tests)

add_executable(vortex_parser_statement_tests
    tests/cpp/parser_statement_tests.cpp
)

target_link_libraries(vortex_parser_statement_tests PRIVATE vortex_compiler)
target_compile_options(vortex_parser_statement_tests PRIVATE -std=c++26)

add_test(NAME parser_statement_tests COMMAND vortex_parser_statement_tests)

add_executable(vortex_parser_declaration_tests
    tests/cpp/parser_declaration_tests.cpp
)

target_link_libraries(vortex_parser_declaration_tests PRIVATE vortex_compiler)
target_compile_options(vortex_parser_declaration_tests PRIVATE -std=c++26)

add_test(NAME parser_declaration_tests COMMAND vortex_parser_declaration_tests)

add_executable(vortex_ast_debug_printer_tests
    tests/cpp/ast_debug_printer_tests.cpp
)

target_link_libraries(vortex_ast_debug_printer_tests PRIVATE vortex_compiler)
target_compile_options(vortex_ast_debug_printer_tests PRIVATE -std=c++26)

add_test(NAME ast_debug_printer_tests COMMAND vortex_ast_debug_printer_tests)

add_executable(vortex_vx_fixture_tests
    tests/cpp/vx_fixture_tests.cpp
)

target_link_libraries(vortex_vx_fixture_tests PRIVATE vortex_compiler)
target_compile_options(vortex_vx_fixture_tests PRIVATE -std=c++26)
target_compile_definitions(vortex_vx_fixture_tests PRIVATE
    VORTEX_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(NAME vx_fixture_tests COMMAND vortex_vx_fixture_tests)

add_custom_target(format
    COMMAND "${CMAKE_SOURCE_DIR}/scripts/format.sh"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Formatting Vortex C++ sources"
)

add_custom_target(tidy-fix
    COMMAND sh -c "scripts/tidy-fix.sh $(find src tests -name '*.cpp')"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Applying clang-tidy fixes"
)
