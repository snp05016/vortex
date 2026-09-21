#pragma once // this is use to prevent inclusion of the same header file
#include "lexer.h"
class Parser {
	public:
		Parser(const char* source,
		       std::size_t length); // this is the constructor for parser
		void parse();		    // this is the main parsing function
	private:
		Lexer lexer;
		void parse_expression();
};