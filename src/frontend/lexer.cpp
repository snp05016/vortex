#include "lexer.h"
#include <string_view>
// :: is the scope resolution op, tells the function that its a member of the class Lexer
Lexer::Lexer(const char* source, std::size_t length)
	: source(source), length(length), position(0) {}
void Lexer::skip_whitespace() {
	// if the current position is less than the length
	// and current character is white space or newline or tab then incremeent
	while (position < length && is_whitespace(source[position])) {
		position++;
	}
}
void Lexer::skip_comment() {
	while (position < length && source[position] != '\n') {
		position++;
	}
}
void Lexer::skip_blk_comment() {
	position += 2; // Skip the /* characters
	while (position + 1 < length && !is_blk_comment_end(source[position], source[position + 1])) {
		position++;
	}
	if (position + 1 < length) {
		position += 2; // skip the */ char
	} else {
		position = length; // if we reach the end of the source without
						   // finding */, we set position to length to indicate EOF.
	}
}
Token Lexer::figure_out_token() {
	const std::size_t start = position;
	// lambda are in the syntax [&] (parameters) { body }
	auto make_token = [&](TokenKind kind) { // lambda funct to quick create a token
		return Token{kind, SourceLocation{start, position - start}, source + start};
	};
	auto check_two = [&](char first, char second, TokenKind kind) {
		if (position + 1 < length && source[position] == first && source[position + 1] == second) {
			position += 2;
			return make_token(kind);
		}
		return Token{TokenKind::INVALID_TOKEN, SourceLocation{start, 0}, nullptr};
	};
	auto has_next = [&]() { // lambda functiosn to check fort he next chara
		return position + 1 < length;
	};
	auto matches = [&](char first, char second) { // string matching ffahhhhhh
		return has_next() && source[position] == first && source[position + 1] == second;
	};
	const char current_char = source[position];
	// ids and kws
	if (is_alpha(current_char)) {
		position++;
		while (position < length && is_alnum(source[position])) {
			position++;
		}
		const std::string_view word(source + start, position - start);
		if (word == "fn")
			return make_token(TokenKind::KW_FN);
		if (word == "let")
			return make_token(TokenKind::KW_LET);
		if (word == "if")
			return make_token(TokenKind::KW_IF);
		if (word == "else")
			return make_token(TokenKind::KW_ELSE);
		if (word == "while")
			return make_token(TokenKind::KW_WHILE);
		if (word == "for")
			return make_token(TokenKind::KW_FOR);
		if (word == "return")
			return make_token(TokenKind::KW_RETURN);
		if (word == "struct")
			return make_token(TokenKind::KW_STRUCT);
		if (word == "mut")
			return make_token(TokenKind::KW_MUT);
		if (word == "in")
			return make_token(TokenKind::KW_IN);
		if (word == "break")
			return make_token(TokenKind::KW_BREAK);
		if (word == "continue")
			return make_token(TokenKind::KW_CONTINUE);
		if (word == "void")
			return make_token(TokenKind::KW_VOID);
		if (word == "bool")
			return make_token(TokenKind::KW_BOOL);
		if (word == "char")
			return make_token(TokenKind::KW_CHAR);
		if (word == "i32")
			return make_token(TokenKind::KW_I32);
		if (word == "u32")
			return make_token(TokenKind::KW_U32);
		if (word == "usize")
			return make_token(TokenKind::KW_USIZE);
		if (word == "f32")
			return make_token(TokenKind::KW_F32);
		if (word == "f64")
			return make_token(TokenKind::KW_F64);
		if (word == "String")
			return make_token(TokenKind::KW_STRING);
		if (word == "true")
			return make_token(TokenKind::LIT_TRUE);
		if (word == "false")
			return make_token(TokenKind::LIT_FALSE);
		return make_token(TokenKind::IDENTIFIER);
	}
	// Integer and floating-point literals
	if (is_digit(current_char)) {
		if (current_char == '0' && has_next() &&
			(source[position + 1] == 'b' || source[position + 1] == 'B')) {
			position += 2;
			const std::size_t binary_start = position;
			while (position < length && (source[position] == '0' || source[position] == '1')) {
				position++;
			}
			if (position == binary_start) {
				return make_token(TokenKind::INVALID_TOKEN);
			}
			return make_token(TokenKind::LIT_INT);
		}
		while (position < length && is_digit(source[position])) {
			position++;
		}
		bool is_float = false;
		if (position + 1 < length && source[position] == '.' && is_digit(source[position + 1])) {
			is_float = true;
			position++;
			while (position < length && is_digit(source[position])) {
				position++;
			}
		}
		if (is_float && position < length && (source[position] == 'e' || source[position] == 'E')) {
			std::size_t exponent_position = position + 1;
			if (exponent_position < length &&
				(source[exponent_position] == '+' || source[exponent_position] == '-')) {
				exponent_position++;
			}
			if (exponent_position < length && is_digit(source[exponent_position])) {
				position = exponent_position + 1;
				while (position < length && is_digit(source[position])) {
					position++;
				}
			}
		}
		return make_token(is_float ? TokenKind::LIT_FLOAT : TokenKind::LIT_INT);
	}
	// String and character literals.
	if (is_string_literal_start(current_char) || is_char_literal_start(current_char)) {
		const char closing_quote = current_char;
		const TokenKind literal_kind =
			is_string_literal_start(current_char) ? TokenKind::LIT_STRING : TokenKind::LIT_CHAR;
		position++;
		bool escaped = false; // to keep track of whether the last character was an esp
		while (position < length) {
			const char character = source[position];
			if (character == '\n' || character == '\r') {
				return make_token(TokenKind::INVALID_TOKEN);
			}
			position++;
			if (escaped) {
				escaped = false;
				continue;
			}
			if (character == '\\') {
				escaped = true;
				continue;
			}
			if (character == closing_quote) {
				return make_token(literal_kind);
			}
		}
		return make_token(TokenKind::INVALID_TOKEN);
	}
	// Multi-character punctuation must be checked before shorter forms.
	if (position + 2 < length && source[position] == '.' && source[position + 1] == '.' &&
		source[position + 2] == '=') {
		position += 3;
		return make_token(TokenKind::PUNC_INCL_RANGE);
	}
	// declaring function return types.
	if (matches('-', '>')) {
		position += 2;
		return make_token(TokenKind::PUNC_ARROW);
	}
	// range iperator
	if (matches('.', '.')) {
		position += 2;
		return make_token(TokenKind::PUNC_RANGE);
	}
	if (is_operator(current_char)) {
		if (Token token = check_two('=', '=', TokenKind::OP_EQUAL);
			token.kind != TokenKind::INVALID_TOKEN)
			return token;
		if (Token token = check_two('!', '=', TokenKind::OP_NOT_EQUAL);
			token.kind != TokenKind::INVALID_TOKEN)
			return token;
		if (Token token = check_two('<', '=', TokenKind::OP_LESS_EQUAL);
			token.kind != TokenKind::INVALID_TOKEN)
			return token;
		if (Token token = check_two('>', '=', TokenKind::OP_GREATER_EQUAL);
			token.kind != TokenKind::INVALID_TOKEN)
			return token;
		if (Token token = check_two('&', '&', TokenKind::OP_LOGICAL_AND);
			token.kind != TokenKind::INVALID_TOKEN)
			return token;
		if (Token token = check_two('|', '|', TokenKind::OP_LOGICAL_OR);
			token.kind != TokenKind::INVALID_TOKEN)
			return token;
		if (Token token = check_two('<', '<', TokenKind::OP_LEFT_SHIFT);
			token.kind != TokenKind::INVALID_TOKEN)
			return token;
		if (Token token = check_two('>', '>', TokenKind::OP_RIGHT_SHIFT);
			token.kind != TokenKind::INVALID_TOKEN)
			return token;
		if (Token token = check_two('+', '=', TokenKind::OP_PLUS_ASSIGN);
			token.kind != TokenKind::INVALID_TOKEN)
			return token;
		if (Token token = check_two('-', '=', TokenKind::OP_MINUS_ASSIGN);
			token.kind != TokenKind::INVALID_TOKEN)
			return token;
		if (Token token = check_two('*', '=', TokenKind::OP_MULTIPLY_ASSIGN);
			token.kind != TokenKind::INVALID_TOKEN)
			return token;
		if (Token token = check_two('/', '=', TokenKind::OP_DIVIDE_ASSIGN);
			token.kind != TokenKind::INVALID_TOKEN)
			return token;
		if (Token token = check_two('%', '=', TokenKind::OP_MODULO_ASSIGN);
			token.kind != TokenKind::INVALID_TOKEN)
			return token;
	}
	if (is_operator(current_char)) {
		position++;
		switch (current_char) {
		case '+':
			return make_token(TokenKind::OP_PLUS);
		case '-':
			return make_token(TokenKind::OP_MINUS);
		case '*':
			return make_token(TokenKind::OP_MULTIPLY);
		case '/':
			return make_token(TokenKind::OP_DIVIDE);
		case '=':
			return make_token(TokenKind::OP_ASSIGN);
		case '<':
			return make_token(TokenKind::OP_LESS);
		case '>':
			return make_token(TokenKind::OP_GREATER);
		case '!':
			return make_token(TokenKind::OP_LOGICAL_NOT);
		case '&':
			return make_token(TokenKind::OP_BITWISE_AND);
		case '|':
			return make_token(TokenKind::OP_BITWISE_OR);
		case '^':
			return make_token(TokenKind::OP_BITWISE_XOR);
		case '~':
			return make_token(TokenKind::OP_BITWISE_NOT);
		case '%':
			return make_token(TokenKind::OP_MODULO);
		default:
			return make_token(TokenKind::INVALID_TOKEN);
		}
	}
	if (is_punctuation(current_char)) {
		position++;
		switch (current_char) {
		case '(':
			return make_token(TokenKind::PUNC_LPAREN);
		case ')':
			return make_token(TokenKind::PUNC_RPAREN);
		case '{':
			return make_token(TokenKind::PUNC_LBRACE);
		case '}':
			return make_token(TokenKind::PUNC_RBRACE);
		case '[':
			return make_token(TokenKind::PUNC_LBRACKET);
		case ']':
			return make_token(TokenKind::PUNC_RBRACKET);
		case ';':
			return make_token(TokenKind::PUNC_SEMICOLON);
		case '.':
			return make_token(TokenKind::PUNC_DOT);
		case ':':
			return make_token(TokenKind::PUNC_COLON);
		case ',':
			return make_token(TokenKind::PUNC_COMMA);
		default:
			return make_token(TokenKind::INVALID_TOKEN);
		}
	}
	// Always consume an invalid character so the lexer keeps making progress.
	position++;
	return make_token(TokenKind::INVALID_TOKEN);
}
Token Lexer::next_token() {
	while (position < length) {
		char current_char = source[position];
		if (is_whitespace(current_char)) {
			skip_whitespace();
			continue;
		}
		if (position + 1 < length && is_line_comment_start(current_char, source[position + 1])) {
			skip_comment();
			continue;
		}
		if (position + 1 < length && is_blk_comment_start(current_char, source[position + 1])) {
			skip_blk_comment();
			continue;
		}
		return figure_out_token();
	}
	// If we reach the end of the source, return an end-of-file token
	Token token;
	token.kind = TokenKind::EOF_TOKEN;
	token.location.start = position;
	token.location.length = 0; // Assuming single character for invalid token
	token.text = nullptr;
	return token;
}
