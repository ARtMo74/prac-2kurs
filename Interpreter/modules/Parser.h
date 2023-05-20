#ifndef INTERPRETER_SYNTAX_ANALYZER_H
#define INTERPRETER_SYNTAX_ANALYZER_H


#include <deque>
#include <stack>
#include <vector>
#include "Lexeme.h"
#include "Scanner.h"

class Parser
{
    Lexeme cur_lex;
    Scanner scanner;
    std::deque <Lexeme> lex_queue;
    std::stack <LexemeType> expr_stack;
    std::vector <Lexeme> poliz;

    static const LexemeType typenames[];
    static const LexemeType expression_parts[];
    static const LexemeType comparisons[];
    static const LexemeType logical_lexemes[];

    void get();
    Lexeme peek();
    static bool check(LexemeType lexeme_type, const LexemeType *arr);
    bool is_assignment();
    bool is_logic_expression();
    void error(std::string &&error_msg);
    void check_type_matching(LexemeType got, LexemeType expected);
    void check_operands();
    void check_in_read();

    void program();
    void definitions();
    void variable(LexemeType type);
    void statements();
    void statement();
    void if_statement();
    void do_while_statement();
    void while_statement();
    void for_statement();
    void read_statement();
    void write_statement();
    void expression();
    void logic_expression();
    void l_term();
    void l_factor();
    void comparison();
    void arithmetic_expression();
    void term();
    void factor();
public:
    explicit Parser(const char *filename);
    void analyze();
};


#endif
