#pragma once
#include "token.h"
#include <iostream>
class Lexer {
  public:
	Lexer(const char* source, std::size_t length);
	Token next_token();
  private:
	const char* source;
	bool is_whitespace(char c) {
		return c == ' ' || c == '\n' || c == '\t';
	}
	bool is_digit(char c) {
		return c >= '0' && c <= '9';
	}
	bool is_alpha(char c) {
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
	}
	bool is_alnum(char c) {
		return is_alpha(c) || is_digit(c);
	}
	bool is_operator(char c) {
		return c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>' ||
			c == '!' || c == '&' || c == '|' || c == '^' || c == '%' || c == '~';
	}
	bool is_punctuation(char c) {
		return c == '(' || c == ')' || c == '{' || c == '}' || c == ';' || c == '.' || c == '[' ||
			c == ']' || c == ':' || c == ',' || c == '-' || c == '>' || c == '.';
	}
	bool is_blk_comment_start(char c1, char c2) {
		return c1 == '/' && c2 == '*';
	}
	bool is_blk_comment_end(char c1, char c2) {
		return c1 == '*' && c2 == '/';
	}
	bool is_line_comment_start(char c1, char c2) {
		return c1 == '/' && c2 == '/';
	}
	bool is_string_literal_start(char c) {
		return c == '"';
	}
	bool is_char_literal_start(char c) {
		return c == '\'';
	}
	void skip_whitespace();
	void skip_comment();
	void skip_blk_comment();
	Token figure_out_token();
	std::size_t length;
	std::size_t position;
};