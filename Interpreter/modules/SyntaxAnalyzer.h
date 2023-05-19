#ifndef INTERPRETER_SYNTAX_ANALYZER_H
#define INTERPRETER_SYNTAX_ANALYZER_H


#include <deque>
#include "Lexeme.h"
#include "LexemeAnalyzer.h"

class SyntaxAnalyzer
{
    Lexeme cur_lex;
    LexemeAnalyzer lex_analyzer;
    std::deque <Lexeme> lex_queue;

    static const LexemeType typenames[];
    static const LexemeType constants[];
    static const LexemeType expression_parts[];
    static const LexemeType comparisons[];
    static const LexemeType logical_lexemes[];

    void get();
    Lexeme peek();
    static bool check(LexemeType lexeme_type, const LexemeType *arr);
    bool is_assignment();

    void program();
    void definitions();
    void variable(LexemeType type);
    void statements();
    void statement();
    void if_statement();
    void do_while_statement();
    void while_statement();
//    void for_statement();
    void read_statement();
    void write_statement();
    void expression();
    void logic_expression();
    void l_term();
    void l_factor();
    void arithmetic_expression();
    void term();
    void factor();
public:
    explicit SyntaxAnalyzer(const char *filename);
    void analyze();
};


#endif
