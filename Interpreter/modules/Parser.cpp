#include <iostream>
#include <iomanip>
#include "Parser.h"
#include "Lexeme.h"
#include "Identifier.h"
#include "Error.h"


const LexemeType Parser::typenames[] = {
        lex_int,
        lex_string,
        lex_boolean,
        lex_null
};
const LexemeType Parser::expression_parts[] = {
        lex_true, lex_false,
        lex_and, lex_or, lex_not,
        lex_plus, lex_minus, lex_mul, lex_div,
        lex_assign,
        lex_less, lex_greater, lex_equal, lex_le, lex_ge, lex_ne,
        lex_left_bracket, lex_right_bracket,
        lex_number, lex_string_literal,
        lex_ident,
        lex_null
};
const LexemeType Parser::comparisons[] = {
        lex_less, lex_greater, lex_equal,
        lex_le, lex_ge, lex_ne,
        lex_null
};
const LexemeType Parser::logical_lexemes[] = {
        lex_true, lex_false,
        lex_and, lex_or, lex_not,
        lex_less, lex_greater, lex_equal,
        lex_le, lex_ge, lex_ne,
        lex_null
};

Parser::Parser(const char *filename): scanner(filename), cur_lex(lex_null, 0) {}
void Parser::error(std::string &&error_msg)
{
    throw Error(error_msg, scanner.line());
}

void Parser::analyze()
{
    get();
    program();
    for (int i = 0; i < poliz.size(); ++i)
        std::cout << std::setw(4) <<  i << ": " << poliz[i] << std::endl;
    if (cur_lex.type == lex_null)
        std::cout << "Program interpreted successfully" << std::endl;
    else
         error("All instructions must be inside program body");
}

void Parser::get()
{
    if (lex_queue.empty())
        cur_lex = scanner.get();
    else
    {
        cur_lex = lex_queue.front();
        lex_queue.pop_front();
    }
}

Lexeme Parser::peek()
{
    if (lex_queue.empty())
        lex_queue.push_back(scanner.get());
    return lex_queue.front();
}

bool Parser::check(LexemeType lexeme_type, const LexemeType *arr)
{
    for (int i = 0; arr[i] != lex_null; i++)
        if (lexeme_type == arr[i]) return true;
    return false;
}

void Parser::program()
{
    if (cur_lex.type == lex_program) get();
    else error("No entry point found");
    if (cur_lex.type == lex_left_curly) get();
    else error("missing '{'");
    definitions();
    statements();
    if (cur_lex.type == lex_right_curly) get();
    else error("missing '}'");
}

void Parser::definitions()
{
    while (check(cur_lex.type, typenames))
    {
        LexemeType type = cur_lex.type;
        get();
        variable(type);
        while(cur_lex.type == lex_comma)
        {
            get();
            variable(type);
        }
        if (cur_lex.type == lex_semicolon) get();
        else error("missing ';'");
    }
}

void Parser::check_type_matching(LexemeType got, LexemeType expected)
{
    if (got == expected)
        get();
    else
        error(std::string("Type mismatch: expected ") + Lexeme::keywords[expected]
              + ", got " + Lexeme::keywords[got] + " instead");
}

void Parser::variable(LexemeType type)
{
    if (cur_lex.type == lex_ident)
    {
        if (!idents[cur_lex.data].is_defined)
            idents[cur_lex.data].is_defined = true;
        else
            error(std::string("Redeclaration of identifier ") + idents[cur_lex.data].get_name());
        idents[cur_lex.data].type = type;
        get();
    }
    else
        error("Identifier expected");
    if (cur_lex.type == lex_assign)
    {
        get();
        switch (cur_lex.type)
        {
            case lex_plus:
            case lex_minus:
                get();
                if (cur_lex.type == lex_number)
                    check_type_matching(lex_int, type);
                else
                    error("Number expected");
                break;
            case lex_number:
                check_type_matching(lex_int, type);
                break;
            case lex_string_literal:
                check_type_matching(lex_string, type);
                break;
            case lex_true:
            case lex_false:
                check_type_matching(lex_boolean, type);
                break;
            default:
                error("Constant expected");
        }
    }
}

void Parser::statements()
{
    while (cur_lex.type != lex_right_curly) statement();
}

void Parser::statement()
{
    switch (cur_lex.type)
    {
        case lex_semicolon:
            get(); break;
        case lex_break:
        case lex_continue:
            poliz.push_back(cur_lex);
            poliz.emplace_back(poliz_fgo);
            get();
            if (cur_lex.type == lex_semicolon) get();
            else error("missing ';'");
            break;
//        case lex_goto:
//            get();
//            if (cur_lex.type == lex_ident) get();
//            else error("Identifier expected");
//            if (cur_lex.type == lex_semicolon) get();
//            else error("missing ';'");
//            break;
//  TODO add support for goto statement
        case lex_if:
            if_statement(); break;
        case lex_do:
            do_while_statement(); break;
        case lex_while:
            while_statement(); break;
        case lex_for:
            for_statement(); break;
        case lex_write:
            write_statement(); break;
        case lex_read:
            read_statement(); break;
        case lex_left_curly:
            get();
            statements();
            if (cur_lex.type == lex_right_curly)
                get();
            else
                error("missing '}'");
            break;
        default:
            expression();
            if (cur_lex.type == lex_semicolon)
            {
                poliz.push_back(cur_lex);
                get();
            }
            else
                error("missing ';'");
    }
}

void Parser::if_statement()
{
    get();
    if (cur_lex.type == lex_left_bracket) get();
    else error("missing '('");
    logic_expression();
    if (cur_lex.type == lex_right_bracket) get();
    else error("missing ')'");
    int place1 = (int)poliz.size();
    poliz.emplace_back();
    poliz.emplace_back(poliz_fgo);
    statement();
    if (cur_lex.type == lex_else)
    {
        get();
        int place2 = (int)poliz.size();
        poliz.emplace_back();
        poliz.emplace_back(poliz_go);
        poliz[place1] = Lexeme(poliz_label, (int)poliz.size());
        statement();
        poliz[place2] = Lexeme(poliz_label, (int)poliz.size());
    }
    else
    {
        poliz[place1] = Lexeme(poliz_label, (int)poliz.size());
    }
}

void Parser::do_while_statement()
{
    get();
    int place1 = (int)poliz.size();
    statement();
    int end = (int)poliz.size();
    if (cur_lex.type == lex_while) get();
    else error("missing 'while'");
    if (cur_lex.type == lex_left_bracket) get();
    else error("missing '('");
    logic_expression();
    if (cur_lex.type == lex_right_bracket) get();
    else error("missing ')'");
    int place2 = (int)poliz.size();
    poliz.emplace_back();
    poliz.emplace_back(poliz_fgo);
    poliz.emplace_back(poliz_label, place1);
    poliz.emplace_back(poliz_go);
    poliz[place2] = Lexeme(poliz_label, (int)poliz.size());
    for (int i = place1; i < end; i++)
    {
        if (poliz[i].type == lex_break)
            poliz[i] = Lexeme(poliz_label, (int)poliz.size());
        if (poliz[i].type == lex_continue)
            poliz[i] = Lexeme(poliz_label, place1);
    }
    if (cur_lex.type == lex_semicolon) get();
    else error("missing ';'");
}
void Parser::while_statement()
{
    get();
    int place1 = (int)poliz.size();
    if (cur_lex.type == lex_left_bracket) get();
    else error("missing '('");
    logic_expression();
    if (cur_lex.type == lex_right_bracket) get();
    else error("missing ')'");
    int place2 = (int)poliz.size();
    poliz.emplace_back();
    poliz.emplace_back(poliz_fgo);
    statement();
    poliz.emplace_back(poliz_label, place1);
    poliz.emplace_back(poliz_go);
    poliz[place2] = Lexeme(poliz_label, (int)poliz.size());
    for (int i = place2 + 2; i < poliz.size() - 2; i++)
    {
        if (poliz[i].type == lex_break)
            poliz[i] = Lexeme(poliz_label, (int)poliz.size());
        if (poliz[i].type == lex_continue)
            poliz[i] = Lexeme(poliz_label, place1);
    }
}
void Parser::for_statement()
{
    get();
    if (cur_lex.type == lex_left_bracket) get();
    else error("missing '('");
    if (cur_lex.type != lex_semicolon)
        expression();
    if (cur_lex.type == lex_semicolon) get();
    else error("missing ';' inside for loop");
    poliz.emplace_back(lex_semicolon, 13);
    int place1 = (int)poliz.size();
    if (cur_lex.type != lex_semicolon)
        logic_expression();
    if (cur_lex.type == lex_semicolon) get();
    else error("missing ';' inside for loop");
    int place2 = (int)poliz.size();
    poliz.emplace_back();
    poliz.emplace_back(poliz_fgo);
    poliz.emplace_back();
    poliz.emplace_back(poliz_go);
    if (cur_lex.type != lex_right_bracket)
        expression();
    if (cur_lex.type == lex_right_bracket) get();
    else error("missing ')'");
    poliz.emplace_back(poliz_label, place1);
    poliz.emplace_back(poliz_go);
    poliz[place2 + 2] = Lexeme(poliz_label, (int)poliz.size());
    statement();
    poliz.emplace_back(poliz_label, place2 + 4);
    poliz.emplace_back(poliz_go);
    poliz[place2] = Lexeme(poliz_label, (int)poliz.size());

}

void Parser::write_statement()
{
    get();
    if (cur_lex.type == lex_left_bracket) get();
    else error("missing '('");
    expression();
    poliz.emplace_back(lex_write, 12);
    while (cur_lex.type == lex_comma)
    {
        get();
        expression();
        poliz.emplace_back(lex_write, 12);
    }
    if (cur_lex.type == lex_right_bracket) get();
    else error("missing ')'");
    if (cur_lex.type == lex_semicolon) get();
    else error("missing ';'");
}

void Parser::check_in_read()
{
    if (!idents[cur_lex.data].is_defined)
        error(std::string("Undefined identifier ") + idents[cur_lex.data].get_name());
    if (idents[cur_lex.data].type != lex_boolean)
    {
        poliz.emplace_back(poliz_address, cur_lex.data);
        get();
    }
    else
        error("Can't input boolean data using 'read'");
}
void Parser::read_statement()
{
    get();
    if (cur_lex.type == lex_left_bracket) get();
    else error("missing '('");
    if (cur_lex.type == lex_ident) check_in_read();
    else error("Identifier expected");
    poliz.emplace_back(lex_read, 11);
    while (cur_lex.type == lex_comma)
    {
        get();
        if (cur_lex.type == lex_ident) check_in_read();
        else error("Identifier expected");
        poliz.emplace_back(lex_read, 11);
    }
    if (cur_lex.type == lex_right_bracket) get();
    else error("missing ')'");
    if (cur_lex.type == lex_semicolon) get();
    else error("missing ';'");
}

bool Parser::is_logic_expression()
{
    bool is_logical = false;
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
            if (!is_logical)
                is_logical = check(cur_lex.type, logical_lexemes) || cur_lex.type == lex_ident && idents[cur_lex.data].type == lex_boolean;
            lex_queue.push_back(cur_lex);
            cur_lex = scanner.get();
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
                || lexeme.type == lex_ident && idents[lexeme.data].type == lex_boolean)
            {
                is_logical = true;
                break;
            }
        }
    }
    return is_logical;
}

void Parser::expression()
{
    if (is_logic_expression())
        logic_expression();
    else
        arithmetic_expression();
}

bool Parser::is_assignment() {
    if (cur_lex.type == lex_ident && peek().type == lex_assign) {
        if (!idents[cur_lex.data].is_defined)
            error(std::string("Undefined identifier ") + idents[cur_lex.data].get_name());
        expr_stack.push(idents[cur_lex.data].type);
        expr_stack.push(lex_assign);
        poliz.emplace_back(poliz_address, cur_lex.data);
        get();
        get();
        return true;
    }
    return false;
}

void Parser::logic_expression()
{
    if (is_assignment())
    {
        logic_expression();
        expr_stack.pop();
        if (expr_stack.top() != lex_boolean)
            error("Operands have different types");
        expr_stack.pop();
        poliz.emplace_back(lex_assign, 4);
    }
    else
    {
        l_term();
        while (cur_lex.type == lex_or)
        {
            get();
            l_term();
            poliz.emplace_back(lex_or, 17);
        }
    }
}

void Parser::l_term()
{
    l_factor();
    while (cur_lex.type == lex_and)
    {
        get();
        l_factor();
        poliz.emplace_back(lex_and, 16);
    }
}

void Parser::l_factor()
{
    switch (cur_lex.type)
    {
        case lex_false:
        case lex_true:
            poliz.push_back(cur_lex);
            get();
            break;
        case lex_not:
            get();
            l_factor();
            poliz.emplace_back(lex_not, 18);
            break;
        case lex_ident:
            if (!idents[cur_lex.data].is_defined)
                error(std::string("Undefined identifier ") + idents[cur_lex.data].get_name());
            if (idents[cur_lex.data].type == lex_boolean)
            {
                poliz.push_back(cur_lex);
                get();
                break;
            }
        case lex_string_literal:
        case lex_number:
            comparison();
            break;
        case lex_left_bracket:
            if (is_logic_expression())
            {
                get();
                logic_expression();
                if (cur_lex.type == lex_right_bracket)
                    get();
                else
                    error("Bad expression");
            }
            else
                comparison();
            break;
        default:
            error("Bad expression");
    }
}

void Parser::comparison()
{
    arithmetic_expression();
    Lexeme cmp = cur_lex;
    if (check(cmp.type, comparisons))
        get();
    else
        error("Bad expression");
    arithmetic_expression();
    LexemeType op1, op2;
    op2 = expr_stack.top(); expr_stack.pop();
    op1 = expr_stack.top(); expr_stack.pop();
    if (op1 != op2)
        error("Can't compare different types");
    poliz.push_back(cmp);
}

void Parser::arithmetic_expression()
{
    if (is_assignment())
    {
        arithmetic_expression();
        check_operands();
        poliz.emplace_back(lex_assign, 4);
    }
    else
    {
        term();
        while (cur_lex.type == lex_plus || cur_lex.type == lex_minus)
        {
            Lexeme operation = cur_lex;
            expr_stack.push(cur_lex.type);
            get();
            term();
            check_operands();
            poliz.push_back(operation);
        }
    }
}

void Parser::term()
{
    factor();
    while (cur_lex.type == lex_mul || cur_lex.type == lex_div)
    {
        Lexeme operation = cur_lex;
        expr_stack.push(cur_lex.type);
        get();
        factor();
        check_operands();
        poliz.push_back(operation);
    }
}

void Parser::factor()
{
    switch (cur_lex.type) {
        case lex_ident:
            if (!idents[cur_lex.data].is_defined)
                error(std::string("Undefined identifier ") + idents[cur_lex.data].get_name());
            expr_stack.push(idents[cur_lex.data].type);
            poliz.push_back(cur_lex);
            get();
            break;
        case lex_number:
        case lex_string_literal:
            expr_stack.push(cur_lex.type == lex_number ? lex_int: lex_string);
            poliz.push_back(cur_lex);
            get();
            break;
        case lex_left_bracket:
            get();
            arithmetic_expression();
            if (cur_lex.type == lex_right_bracket)
                get();
            else
                error("Bad expression");
            break;
        default:
            error("Bad expression");
    }
}

void Parser::check_operands()
{
    LexemeType op2 = expr_stack.top(); expr_stack.pop();
    LexemeType operation = expr_stack.top(); expr_stack.pop();
    LexemeType op1 = expr_stack.top(); expr_stack.pop();
    if (op1 != op2)
        error("Operands have different types");
    if (operation == lex_mul || operation == lex_div || operation == lex_minus)
    {
        if (op1 == lex_string)
            error("Can only add strings");
        expr_stack.push(lex_int);
    }
    else if (operation == lex_plus || operation == lex_assign)
        expr_stack.push(op1);
}
