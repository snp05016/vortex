#include "parser.h"
#include "ast.h"
using namespace std;
Parser::Parser(const char* source, std::size_t length) : lexer(source, length) {}
void Parser::parse() {
        while (true){
                Token tok = lexer.next_token();
                if (tok.kind == TokenKind::KW_LET) {
                        cout << "Parsed a 'let' keyword\n";
                }
        }
}

// this function is just for parsing expressions,
// for example) 1 + 2 * 3, (4 - 5) / 6, etc.
// or if (value < 10)
//       ^^^^^^^^^^^
void Parser::parse_expression() {

}