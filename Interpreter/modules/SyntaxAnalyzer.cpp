#include <iostream>
#include "SyntaxAnalyzer.h"
#include "Lexeme.h"
#include "Identifier.h"


const LexemeType SyntaxAnalyzer::typenames[] = {
        lex_int, lex_real,
        lex_string, lex_boolean,
        lex_null
};
const LexemeType SyntaxAnalyzer::constants[] = {
        lex_true,
        lex_false,
        lex_real_number,
        lex_integer_number,
        lex_string_literal,
        lex_null
};
const LexemeType SyntaxAnalyzer::expression_parts[] = {
        lex_true, lex_false,
        lex_and, lex_or, lex_not,
        lex_plus, lex_minus, lex_mul, lex_div,
        lex_assign,
        lex_less, lex_greater, lex_equal, lex_le, lex_ge, lex_ne,
        lex_left_bracket, lex_right_bracket,
        lex_integer_number, lex_real_number, lex_string_literal,
        lex_ident,
        lex_null
};
const LexemeType SyntaxAnalyzer::comparisons[] = {
        lex_less, lex_greater, lex_equal,
        lex_le, lex_ge, lex_ne,
        lex_null
};
const LexemeType SyntaxAnalyzer::logical_lexemes[] = {
        lex_true, lex_false,
        lex_and, lex_or, lex_not,
        lex_less, lex_greater, lex_equal,
        lex_le, lex_ge, lex_ne,
        lex_null
};

SyntaxAnalyzer::SyntaxAnalyzer(const char *filename): lex_analyzer(filename), cur_lex(lex_null, 0) {}

void SyntaxAnalyzer::analyze()
{
    get();
    program();
    if (cur_lex.type == lex_null) std::cout << "OK" << std::endl;
    else throw "Outside program body!";
}

void SyntaxAnalyzer::get()
{
    if (lex_queue.empty())
        cur_lex = lex_analyzer.get();
    else
    {
        cur_lex = lex_queue.front();
        lex_queue.pop_front();
    }
}

Lexeme SyntaxAnalyzer::peek()
{
    if (lex_queue.empty())
        lex_queue.push_back(lex_analyzer.get());
    return lex_queue.front();
}

bool SyntaxAnalyzer::check(LexemeType lexeme_type, const LexemeType *arr)
{
    for (int i = 0; arr[i] != lex_null; i++)
        if (lexeme_type == arr[i]) return true;
    return false;
}

void SyntaxAnalyzer::program()
{
    if (cur_lex.type == lex_program) get();
    else throw "No entry point found";
    if (cur_lex.type == lex_left_curly) get();
    else throw "No parenthesis found";
    definitions();
    statements();
    if (cur_lex.type == lex_right_curly) get();
    else throw "No parenthesis found";
}

void SyntaxAnalyzer::definitions()
{
    while (check(cur_lex.type, typenames))
    {
        LexemeType type = cur_lex.type;
        get();
        variable(type);
        while(cur_lex.type == lex_comma) { get(); variable(type); }
        if (cur_lex.type == lex_semicolon) get();
        else throw "Semicolon expected after variable definition!";
    }
}

void SyntaxAnalyzer::variable(LexemeType type)
{
    if (cur_lex.type == lex_ident)
    {
        idents[cur_lex.data_i].type = type;
        idents[cur_lex.data_i].is_defined = true;
        get();
    }
    else throw "Identifier expected!";
    if (cur_lex.type == lex_assign)
    {
        get();
        if (cur_lex.type == lex_plus || cur_lex.type == lex_minus)
        {
            get();
            if (cur_lex.type == lex_integer_number) get();
            else throw "Number expected!";
        }
        else if (check(cur_lex.type, constants)) get();
        else throw "Constant expected!";
    }
}

void SyntaxAnalyzer::statements()
{
    while (cur_lex.type != lex_right_curly) statement();
}

void SyntaxAnalyzer::statement()
{
    switch (cur_lex.type)
    {
        case lex_semicolon:
            get(); break;
        case lex_break:
        case lex_continue:
            get();
            if (cur_lex.type == lex_semicolon) get();
            else throw "NO ;";
            break;
        case lex_goto:
            get();
            if (cur_lex.type == lex_ident) get();
            else throw "NO ident";
            if (cur_lex.type == lex_semicolon) get();
            else throw "NO ;";
            break;
        case lex_if:
            if_statement(); break;
        case lex_do:
            do_while_statement(); break;
        case lex_while:
            while_statement(); break;
        case lex_write:
            write_statement(); break;
        case lex_read:
            read_statement(); break;
        case lex_left_curly:
            get();
            statements();
            if (cur_lex.type == lex_right_curly) get();
            else throw "Missing } in compound operator";
            break;
        default:
            expression();
            if (cur_lex.type == lex_semicolon)
                get();
            else
                throw "NO ;";
    }
}

void SyntaxAnalyzer::if_statement()
{
    get();
    if (cur_lex.type == lex_left_bracket) get();
    else throw "NO (";
    logic_expression();
    if (cur_lex.type == lex_right_bracket) get();
    else throw "NO )";
    statement();
    if (cur_lex.type == lex_else)
    {
        get();
        statement();
    }
}

void SyntaxAnalyzer::do_while_statement()
{
    get();
    statement();
    if (cur_lex.type == lex_while) get();
    else throw "NO 'while'";
    if (cur_lex.type == lex_left_bracket) get();
    else throw "NO (";
    logic_expression();
    if (cur_lex.type == lex_right_bracket) get();
    else throw "NO )";
    if (cur_lex.type == lex_semicolon) get();
    else throw "Missing ; after statement";
}

void SyntaxAnalyzer::while_statement()
{
    get();
    if (cur_lex.type == lex_left_bracket) get();
    else throw "NO (";
    logic_expression();
    if (cur_lex.type == lex_right_bracket) get();
    else throw "NO )";
    statement();
}

void SyntaxAnalyzer::write_statement()
{
    get();
    if (cur_lex.type == lex_left_bracket) get();
    else throw "NO (";
    expression();
    while (cur_lex.type == lex_comma)
    {
        get();
        expression();
    }
    if (cur_lex.type == lex_right_bracket) get();
    else throw "NO )";
    if (cur_lex.type == lex_semicolon) get();
    else throw "Missing ; after statement";
}

void SyntaxAnalyzer::read_statement()
{
    get();
    if (cur_lex.type == lex_left_bracket) get();
    else throw "No bracket";
    if (cur_lex.type == lex_ident) get();
    else throw "Must be an identifier";
    while (cur_lex.type == lex_comma)
    {
        get();
        if (cur_lex.type == lex_ident) get();
        else throw "Must be an identifier";
    }
    if (cur_lex.type == lex_right_bracket) get();
    else throw "No bracket";
    if (cur_lex.type == lex_semicolon) get();
    else throw "Missing ; after statement";
}

void SyntaxAnalyzer::expression()
{
    bool is_logical = false;
    int bracket_counter = 0;
    while (check(cur_lex.type, expression_parts))
    {
        if (cur_lex.type == lex_right_bracket)
        {
            if (bracket_counter == 0)
                break;
            else
                --bracket_counter;
        }
        if (cur_lex.type == lex_left_bracket)
            ++bracket_counter;
        if (!is_logical)
            is_logical = check(cur_lex.type, logical_lexemes) || cur_lex.type == lex_ident && idents[cur_lex.data_i].type == lex_boolean;
        lex_queue.push_back(cur_lex);
        cur_lex = lex_analyzer.get();
    }
    lex_queue.push_back(cur_lex);
    cur_lex = lex_queue.front();
    lex_queue.pop_front();
    if (is_logical)
        logic_expression();
    else
        arithmetic_expression();
}

bool SyntaxAnalyzer::is_assignment() {
    if (cur_lex.type == lex_ident && peek().type == lex_assign) {
        get();
        get();
        return true;
    }
    return false;
}

void SyntaxAnalyzer::logic_expression()
{
    if (is_assignment()) logic_expression();
    else
    {
        l_term();
        while (cur_lex.type == lex_or)
        {
            get();
            l_term();
        }
    }
}

void SyntaxAnalyzer::l_term()
{
    l_factor();
    while (cur_lex.type == lex_and)
    {
        get();
        l_factor();
    }
}

void SyntaxAnalyzer::l_factor()
{
    switch (cur_lex.type)
    {
        case lex_false:
        case lex_true:
            get(); break;
        case lex_not:
            get();
            l_factor();
            break;
        case lex_ident:
            if (idents[cur_lex.data_i].type == lex_boolean)
            {
                get();
                break;
            }
        case lex_string_literal:
        case lex_integer_number:
        case lex_real_number:
            arithmetic_expression();
            if (check(cur_lex.type, comparisons))
                get();
            else
                throw "Expected comparison";
            arithmetic_expression();
            break;
        case lex_left_bracket:
        {
            bool is_logic = false;
            int bracket_counter = 0;
            if (lex_queue.empty())
            {
                while (check(cur_lex.type, expression_parts))
                {
                    if (cur_lex.type == lex_right_bracket)
                    {
                        if (bracket_counter == 0)
                            break;
                        else
                            --bracket_counter;
                    }
                    if (cur_lex.type == lex_left_bracket)
                        ++bracket_counter;
                    if (!is_logic)
                        is_logic = check(cur_lex.type, logical_lexemes) || cur_lex.type == lex_ident && idents[cur_lex.data_i].type == lex_boolean;
                    lex_queue.push_back(cur_lex);
                    cur_lex = lex_analyzer.get();
                }
                lex_queue.push_back(cur_lex);
                cur_lex = lex_queue.front();
                lex_queue.pop_front();
            }
            else
            {
                for (const Lexeme lexeme: lex_queue)
                {
                    if (lexeme.type == lex_left_bracket)
                        ++bracket_counter;
                    if (lexeme.type == lex_right_bracket)
                    {
                        if (bracket_counter == 0)
                            break;
                        else
                            --bracket_counter;
                    }
                    if (check(lexeme.type, logical_lexemes)
                        || lexeme.type == lex_ident && idents[lexeme.data_i].type == lex_boolean)
                    {
                        is_logic = true;
                        break;
                    }
                }
            }
            if (is_logic)
            {
                get();
                logic_expression();
                if (cur_lex.type == lex_right_bracket)
                    get();
                else
                    throw "NO )";
            }
            else
            {
                arithmetic_expression();
                if (check(cur_lex.type, comparisons))
                    get();
                else
                    throw "Comparison expected";
                arithmetic_expression();
            }
        }
        break;
        default:
            throw "Bad l_factor";
    }
}
void SyntaxAnalyzer::arithmetic_expression()
{
    if (is_assignment())
        arithmetic_expression();
    else
    {
        term();
        while (cur_lex.type == lex_plus || cur_lex.type == lex_minus)
        {
            get();
            term();
        }
    }
}

void SyntaxAnalyzer::term()
{
    factor();
    while (cur_lex.type == lex_mul || cur_lex.type == lex_div)
    {
        get();
        factor();
    }
}

void SyntaxAnalyzer::factor()
{
    if (check(cur_lex.type, constants) || cur_lex.type == lex_ident)
        get();
    else if (cur_lex.type == lex_left_bracket)
    {
        get();
        arithmetic_expression();
        if (cur_lex.type == lex_right_bracket)
            get();
        else
            throw "NO )";
    }
    else throw "wrong factor";
}
