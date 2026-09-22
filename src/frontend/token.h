#pragma once // pragma is used for header guards, to prevent multiple inclusions
             // of the same header
// file
#include <cstddef> // for size_t
#include <string>
enum class TokenKind { // Using an enum because the token kinds are fixed.
  // Keywords
  KW_FN,     // "fn"
  KW_LET,    // "let"
  KW_IF,     // "if"
  KW_ELSE,   // "else"
  KW_WHILE,  // "while"
  KW_FOR,    // "for"
  KW_RETURN, // "return"
  KW_STRUCT,
  KW_MUT,
  KW_IN,
  KW_BREAK,
  KW_CONTINUE,
  KW_VOID,
  KW_BOOL,
  KW_CHAR,
  KW_I32,
  KW_U32,   // 32 bit unsigned integer
  KW_USIZE, // represents the size of memory pointer
  KW_F32,
  KW_F64,
  KW_STRING,
  // Operators
  OP_PLUS,            // +
  OP_MINUS,           // -
  OP_MULTIPLY,        // *
  OP_DIVIDE,          // /
  OP_ASSIGN,          // =
  OP_EQUAL,           // ==
  OP_NOT_EQUAL,       // !=
  OP_LESS,            // <
  OP_GREATER,         // >
  OP_LESS_EQUAL,      // <=
  OP_GREATER_EQUAL,   // >=
  OP_LOGICAL_AND,     // &&
  OP_LOGICAL_OR,      // ||
  OP_LOGICAL_NOT,     // !
  OP_BITWISE_AND,     // &
  OP_BITWISE_OR,      // |
  OP_BITWISE_XOR,     // ^
  OP_BITWISE_NOT,     // ~
  OP_LEFT_SHIFT,      // <<
  OP_RIGHT_SHIFT,     // >>
  OP_MODULO,          // %
  OP_PLUS_ASSIGN,     // +=
  OP_MINUS_ASSIGN,    // -=
  OP_MULTIPLY_ASSIGN, // *=
  OP_DIVIDE_ASSIGN,   // /=
  OP_MODULO_ASSIGN,   // %=
  // Punctuation
  PUNC_LPAREN,    // (
  PUNC_RPAREN,    // )
  PUNC_LBRACE,    // {
  PUNC_RBRACE,    // }
  PUNC_SEMICOLON, // ;
  // Punctuation like [ ] , : .
  PUNC_DOT,                  // .
  PUNC_LBRACKET,             // [
  PUNC_RBRACKET,             // ]
  PUNC_COLON,                // :
  PUNC_COMMA,                // ,
  PUNC_ARROW,                // ->
  PUNC_RANGE,                // ..
  PUNC_INCL_RANGE,           // ..=
  PUNC_INCL_BLK_COMMENT,     // /*
  PUNC_INCL_BLK_COMMENT_END, // */
  // Identifiers
  IDENTIFIER,
  // LITERALS
  LIT_STRING,
  LIT_INT,
  LIT_FLOAT,
  LIT_CHAR,
  LIT_TRUE,
  LIT_FALSE,
  // invalid
  INVALID_TOKEN,
  // End of file
  EOF_TOKEN,
};
// tells us the size of the line of hte code and gives the locaitons and the
// start as well
struct SourceLocation {
  std::size_t start;
  std::size_t length;
};
// repreesnts a token in the source code with its kind and its location
struct Token {
  TokenKind kind;
  SourceLocation location;
  const char *text; // pointer to the text of the token
};
