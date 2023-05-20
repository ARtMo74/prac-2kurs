#include <iostream>
#include <vector>
#include "Lexeme.h"
#include "Identifier.h"

Lexeme::Lexeme(LexemeType t, int i) : type(t), data(i) {}
const char* Lexeme::keywords[] = {
        "program",
        "int", "string", "boolean",
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
        ",", ":", ";",
        "(", ")", "{", "}", nullptr
};

extern std::vector<std::string> str_literals;

std::ostream& operator<<(std::ostream& stream, Lexeme lex)
{
    if (lex.type < 19)
        stream << Lexeme::keywords[lex.data];
    else if (lex.type < 37)
        stream << Lexeme::separators[lex.data];
    else if (lex.type == lex_number || lex.type == poliz_label)
        stream << lex.data;
    else if (lex.type == lex_string_literal)
        stream << str_literals[lex.data];
    else if (lex.type == lex_ident)
        stream << idents[lex.data].get_name();
    else if (lex.type == poliz_address)
        stream << "@" << idents[lex.data].get_name();
    else if (lex.type == poliz_go)
        stream << "!";
    else if (lex.type == poliz_fgo)
        stream << "!F";
    return stream;
}
// TODO fix number to var, or not...
