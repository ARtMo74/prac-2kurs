//#include <iostream>
//#include <vector>
#include "Lexeme.h"

Lexeme::Lexeme(LexemeType t, int i) : type(t), data_i(i) {}
Lexeme::Lexeme(LexemeType t, double r) : type(t), data_d(r) {}
const char* Lexeme::keywords[] = {
        "program",
        "int", "real", "string", "boolean",
        "true", "false",
        "if", "else",
        "do", "while", "for",
        "read", "write",
        "goto", "break", "continue",
        "and", "or", "not", nullptr
};
const char* Lexeme::separators[] = {
        "+", "-", "*", "/",
        "=",
        "<", ">", "==", "<=", ">=", "!=",
        ",", ".", ":", ";",
        "(", ")", "{", "}", nullptr
};

//extern std::vector<std::string> str_literals;
//extern std::vector<std::string> idents;

//std::ostream& operator<<(std::ostream& stream, Lexeme lex)
//{
//    stream << lex.type <<": ";
//    if (lex.type < 20)
//        stream << Lexeme::keywords[lex.data_i];
//    else if (lex.type < 39)
//        stream << Lexeme::separators[lex.data_i];
//    else if (lex.type == lex_integer_number)
//        stream << lex.data_i;
//    else if (lex.type == lex_real_number)
//        stream << lex.data_d;
//    else if (lex.type == lex_string_literal)
//        stream << str_literals[lex.data_i];
//    else
//        stream << idents[lex.data_i];
//    return stream;
//}
// TODO fix number to var, or not...
