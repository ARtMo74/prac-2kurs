#ifndef INTERPRETER_LEXEME_H
#define INTERPRETER_LEXEME_H


#include <iosfwd>
enum LexemeType
{
    lex_program,
    lex_int, lex_string, lex_boolean,
    lex_true, lex_false,
    lex_if, lex_else,
    lex_do, lex_while, lex_for,
    lex_read, lex_write,
    lex_goto, lex_break, lex_continue,
    lex_and, lex_or, lex_not,
// 19
    lex_plus, lex_minus, lex_mul, lex_div,
    lex_assign,
    lex_less, lex_greater, lex_equal, lex_le, lex_ge, lex_ne,
    lex_comma, lex_colon, lex_semicolon,
    lex_left_bracket, lex_right_bracket,
    lex_left_curly, lex_right_curly,
// 37
    lex_number, lex_string_literal,
    lex_ident,
// POLIZ
    poliz_address,
    poliz_label,
    poliz_go,
    poliz_fgo,

    lex_null
};

struct Lexeme
{
    static const char* keywords[];
    static const char* separators[];
    LexemeType type;
    int data;
    explicit Lexeme(LexemeType t=lex_null, int i=0);
};
std::ostream& operator<<(std::ostream& stream, Lexeme lex);

#endif
