# CMake generated Testfile for 
# Source directory: /Users/saumya/vortex
# Build directory: /Users/saumya/vortex/build-asan
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(lexer_tests "/Users/saumya/vortex/build-asan/vortex_lexer_tests")
set_tests_properties(lexer_tests PROPERTIES  _BACKTRACE_TRIPLES "/Users/saumya/vortex/CMakeLists.txt;75;add_test;/Users/saumya/vortex/CMakeLists.txt;0;")
add_test(parser_expression_tests "/Users/saumya/vortex/build-asan/vortex_parser_expression_tests")
set_tests_properties(parser_expression_tests PROPERTIES  _BACKTRACE_TRIPLES "/Users/saumya/vortex/CMakeLists.txt;84;add_test;/Users/saumya/vortex/CMakeLists.txt;0;")
add_test(parser_type_tests "/Users/saumya/vortex/build-asan/vortex_parser_type_tests")
set_tests_properties(parser_type_tests PROPERTIES  _BACKTRACE_TRIPLES "/Users/saumya/vortex/CMakeLists.txt;93;add_test;/Users/saumya/vortex/CMakeLists.txt;0;")
add_test(parser_statement_tests "/Users/saumya/vortex/build-asan/vortex_parser_statement_tests")
set_tests_properties(parser_statement_tests PROPERTIES  _BACKTRACE_TRIPLES "/Users/saumya/vortex/CMakeLists.txt;102;add_test;/Users/saumya/vortex/CMakeLists.txt;0;")
add_test(parser_declaration_tests "/Users/saumya/vortex/build-asan/vortex_parser_declaration_tests")
set_tests_properties(parser_declaration_tests PROPERTIES  _BACKTRACE_TRIPLES "/Users/saumya/vortex/CMakeLists.txt;111;add_test;/Users/saumya/vortex/CMakeLists.txt;0;")
add_test(ast_debug_printer_tests "/Users/saumya/vortex/build-asan/vortex_ast_debug_printer_tests")
set_tests_properties(ast_debug_printer_tests PROPERTIES  _BACKTRACE_TRIPLES "/Users/saumya/vortex/CMakeLists.txt;120;add_test;/Users/saumya/vortex/CMakeLists.txt;0;")
add_test(scope_resolver_tests "/Users/saumya/vortex/build-asan/vortex_scope_resolver_tests")
set_tests_properties(scope_resolver_tests PROPERTIES  _BACKTRACE_TRIPLES "/Users/saumya/vortex/CMakeLists.txt;129;add_test;/Users/saumya/vortex/CMakeLists.txt;0;")
add_test(vx_fixture_tests "/Users/saumya/vortex/build-asan/vortex_vx_fixture_tests")
set_tests_properties(vx_fixture_tests PROPERTIES  _BACKTRACE_TRIPLES "/Users/saumya/vortex/CMakeLists.txt;141;add_test;/Users/saumya/vortex/CMakeLists.txt;0;")
add_test(diagnostic_listener_tests "/Users/saumya/vortex/build-asan/vortex_diagnostic_listener_tests")
set_tests_properties(diagnostic_listener_tests PROPERTIES  _BACKTRACE_TRIPLES "/Users/saumya/vortex/CMakeLists.txt;150;add_test;/Users/saumya/vortex/CMakeLists.txt;0;")
