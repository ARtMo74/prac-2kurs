#ifndef INTERPRETER_LEXEME_H
#define INTERPRETER_LEXEME_H


#include <iosfwd>
enum LexemeType
{
    lex_program,
    lex_int, lex_real, lex_string, lex_boolean,
    lex_true, lex_false,
    lex_if, lex_else,
    lex_do, lex_while, lex_for,
    lex_read, lex_write,
    lex_goto, lex_break, lex_continue,
    lex_and, lex_or, lex_not,
// 20
    lex_plus, lex_minus, lex_mul, lex_div,
    lex_assign,
    lex_less, lex_greater, lex_equal, lex_le, lex_ge, lex_ne,
    lex_comma, lex_period, lex_colon, lex_semicolon,
    lex_left_bracket, lex_right_bracket,
    lex_left_curly, lex_right_curly,
// 39
    lex_integer_number, lex_real_number, lex_string_literal,
    lex_ident,

    lex_null
};

struct Lexeme
{
    static const char* keywords[];
    static const char* separators[];
    LexemeType type;
    union
    {
        int data_i;
        double data_d;
    };
    Lexeme(LexemeType t, int i);
    Lexeme(LexemeType t, double r);
};
//std::ostream& operator<<(std::ostream& stream, Lexeme lex);

#endif
