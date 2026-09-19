/*
these are the tests for the lexer, they are written by LLMs
*/
#include "frontend/lexer.h"
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
namespace {
struct ExpectedToken {
	TokenKind kind;
	std::string_view text;
};
int failures = 0;
void fail(std::string_view test_name, const std::string& message) {
	std::cerr << "[FAIL] " << test_name << ": " << message << '\n';
	failures++;
}
void expect_tokens(std::string_view test_name, std::string_view source,
				   std::initializer_list<ExpectedToken> expected_tokens) {
	Lexer lexer(source.data(), source.size());
	std::size_t previous_end = 0;
	std::size_t token_index = 0;
	for (const ExpectedToken& expected : expected_tokens) {
		const Token actual = lexer.next_token();
		if (actual.kind != expected.kind) {
			fail(test_name, "token " + std::to_string(token_index) + " has kind " +
								std::to_string(static_cast<int>(actual.kind)) + ", expected " +
								std::to_string(static_cast<int>(expected.kind)));
		}
		if (actual.location.start > source.size() ||
			actual.location.length > source.size() - actual.location.start) {
			fail(test_name, "token span falls outside the source text");
			return;
		}
		const std::string_view actual_text(source.data() + actual.location.start,
										   actual.location.length);
		if (actual_text != expected.text) {
			fail(test_name, "token " + std::to_string(token_index) + " has text \"" +
								std::string(actual_text) + "\", expected \"" +
								std::string(expected.text) + "\"");
		}
		if (actual.location.start < previous_end) {
			fail(test_name, "token spans move backwards or overlap");
		}
		if (actual.text != source.data() + actual.location.start) {
			fail(test_name, "token text pointer does not match its source span");
		}
		previous_end = actual.location.start + actual.location.length;
		token_index++;
	}
	const Token eof = lexer.next_token();
	if (eof.kind != TokenKind::EOF_TOKEN) {
		fail(test_name, "lexer produced an unexpected token after the expected sequence");
	}
	if (eof.location.start != source.size() || eof.location.length != 0) {
		fail(test_name, "EOF token has the wrong source span");
	}
}
void test_keywords() {
	expect_tokens("keywords",
				  "fn struct let mut return if else while for in break continue "
				  "void bool char i32 u32 usize f32 f64 String true false name _value value2",
				  {
					  {TokenKind::KW_FN, "fn"},			 {TokenKind::KW_STRUCT, "struct"},
					  {TokenKind::KW_LET, "let"},		 {TokenKind::KW_MUT, "mut"},
					  {TokenKind::KW_RETURN, "return"},	 {TokenKind::KW_IF, "if"},
					  {TokenKind::KW_ELSE, "else"},		 {TokenKind::KW_WHILE, "while"},
					  {TokenKind::KW_FOR, "for"},		 {TokenKind::KW_IN, "in"},
					  {TokenKind::KW_BREAK, "break"},	 {TokenKind::KW_CONTINUE, "continue"},
					  {TokenKind::KW_VOID, "void"},		 {TokenKind::KW_BOOL, "bool"},
					  {TokenKind::KW_CHAR, "char"},		 {TokenKind::KW_I32, "i32"},
					  {TokenKind::KW_U32, "u32"},		 {TokenKind::KW_USIZE, "usize"},
					  {TokenKind::KW_F32, "f32"},		 {TokenKind::KW_F64, "f64"},
					  {TokenKind::KW_STRING, "String"},	 {TokenKind::LIT_TRUE, "true"},
					  {TokenKind::LIT_FALSE, "false"},	 {TokenKind::IDENTIFIER, "name"},
					  {TokenKind::IDENTIFIER, "_value"}, {TokenKind::IDENTIFIER, "value2"},
				  });
}
void test_numbers() {
	expect_tokens("numbers", "0 42 0b0 0b1010 3.14 1.5e4 2.5E+8 9.0e-2 0..10 0..=10",
				  {
					  {TokenKind::LIT_INT, "0"},
					  {TokenKind::LIT_INT, "42"},
					  {TokenKind::LIT_INT, "0b0"},
					  {TokenKind::LIT_INT, "0b1010"},
					  {TokenKind::LIT_FLOAT, "3.14"},
					  {TokenKind::LIT_FLOAT, "1.5e4"},
					  {TokenKind::LIT_FLOAT, "2.5E+8"},
					  {TokenKind::LIT_FLOAT, "9.0e-2"},
					  {TokenKind::LIT_INT, "0"},
					  {TokenKind::PUNC_RANGE, ".."},
					  {TokenKind::LIT_INT, "10"},
					  {TokenKind::LIT_INT, "0"},
					  {TokenKind::PUNC_INCL_RANGE, "..="},
					  {TokenKind::LIT_INT, "10"},
				  });
}
void test_literals() {
	expect_tokens("literals", R"("hello" "line\nnext" "quote: \"" 'V' '\n' '\'' true false)",
				  {
					  {TokenKind::LIT_STRING, R"("hello")"},
					  {TokenKind::LIT_STRING, R"("line\nnext")"},
					  {TokenKind::LIT_STRING, R"("quote: \"")"},
					  {TokenKind::LIT_CHAR, "'V'"},
					  {TokenKind::LIT_CHAR, "'\\n'"},
					  {TokenKind::LIT_CHAR, "'\\''"},
					  {TokenKind::LIT_TRUE, "true"},
					  {TokenKind::LIT_FALSE, "false"},
				  });
}
void test_operators_and_punctuation() {
	expect_tokens("operators and punctuation",
				  "+ - * / % = == != < <= > >= && || ! & | ^ ~ << >> "
				  "+= -= *= /= %= -> .. ..= ( ) { } [ ] ; . : ,",
				  {
					  {TokenKind::OP_PLUS, "+"},		   {TokenKind::OP_MINUS, "-"},
					  {TokenKind::OP_MULTIPLY, "*"},	   {TokenKind::OP_DIVIDE, "/"},
					  {TokenKind::OP_MODULO, "%"},		   {TokenKind::OP_ASSIGN, "="},
					  {TokenKind::OP_EQUAL, "=="},		   {TokenKind::OP_NOT_EQUAL, "!="},
					  {TokenKind::OP_LESS, "<"},		   {TokenKind::OP_LESS_EQUAL, "<="},
					  {TokenKind::OP_GREATER, ">"},		   {TokenKind::OP_GREATER_EQUAL, ">="},
					  {TokenKind::OP_LOGICAL_AND, "&&"},   {TokenKind::OP_LOGICAL_OR, "||"},
					  {TokenKind::OP_LOGICAL_NOT, "!"},	   {TokenKind::OP_BITWISE_AND, "&"},
					  {TokenKind::OP_BITWISE_OR, "|"},	   {TokenKind::OP_BITWISE_XOR, "^"},
					  {TokenKind::OP_BITWISE_NOT, "~"},	   {TokenKind::OP_LEFT_SHIFT, "<<"},
					  {TokenKind::OP_RIGHT_SHIFT, ">>"},   {TokenKind::OP_PLUS_ASSIGN, "+="},
					  {TokenKind::OP_MINUS_ASSIGN, "-="},  {TokenKind::OP_MULTIPLY_ASSIGN, "*="},
					  {TokenKind::OP_DIVIDE_ASSIGN, "/="}, {TokenKind::OP_MODULO_ASSIGN, "%="},
					  {TokenKind::PUNC_ARROW, "->"},	   {TokenKind::PUNC_RANGE, ".."},
					  {TokenKind::PUNC_INCL_RANGE, "..="}, {TokenKind::PUNC_LPAREN, "("},
					  {TokenKind::PUNC_RPAREN, ")"},	   {TokenKind::PUNC_LBRACE, "{"},
					  {TokenKind::PUNC_RBRACE, "}"},	   {TokenKind::PUNC_LBRACKET, "["},
					  {TokenKind::PUNC_RBRACKET, "]"},	   {TokenKind::PUNC_SEMICOLON, ";"},
					  {TokenKind::PUNC_DOT, "."},		   {TokenKind::PUNC_COLON, ":"},
					  {TokenKind::PUNC_COMMA, ","},
				  });
}
void test_comments_and_whitespace() {
	expect_tokens("comments and whitespace",
				  " \t\n// line comment\n/* block\ncomment */ let/*middle*/value",
				  {
					  {TokenKind::KW_LET, "let"},
					  {TokenKind::IDENTIFIER, "value"},
				  });
}
void test_invalid_input() {
	expect_tokens("invalid character makes progress", "@ let",
				  {
					  {TokenKind::INVALID_TOKEN, "@"},
					  {TokenKind::KW_LET, "let"},
				  });
	expect_tokens("unterminated string", "\"unfinished",
				  {
					  {TokenKind::INVALID_TOKEN, "\"unfinished"},
				  });
	expect_tokens("binary literal needs a digit", "0b",
				  {
					  {TokenKind::INVALID_TOKEN, "0b"},
				  });
}
void test_empty_input_and_stable_eof() {
	constexpr std::string_view source = "";
	Lexer lexer(source.data(), source.size());
	const Token first = lexer.next_token();
	const Token second = lexer.next_token();
	if (first.kind != TokenKind::EOF_TOKEN || second.kind != TokenKind::EOF_TOKEN) {
		fail("stable EOF", "empty input did not repeatedly produce EOF");
	}
	if (first.location.start != 0 || first.location.length != 0 || second.location.start != 0 ||
		second.location.length != 0) {
		fail("stable EOF", "EOF position changed between calls");
	}
}
} // namespace
int main() {
	test_keywords();
	test_numbers();
	test_literals();
	test_operators_and_punctuation();
	test_comments_and_whitespace();
	test_invalid_input();
	test_empty_input_and_stable_eof();
	if (failures != 0) {
		std::cerr << failures << " lexer test(s) failed\n";
		return EXIT_FAILURE;
	}
	std::cout << "All lexer tests passed\n";
	return EXIT_SUCCESS;
}
