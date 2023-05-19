#include <vector>
#include <cctype>
#include "LexemeAnalyzer.h"
#include "Identifier.h"

extern std::vector<std::string> str_literals;

int LexemeAnalyzer::find(const std::string &s, const char** str_array)
{
    int i = 0;
    while (str_array[i] != nullptr && std::string(str_array[i]) != s)
        i++;
    if (str_array[i] == nullptr)
        return -1;
    else
        return i;
}

bool LexemeAnalyzer::is_separator(char c)
{
    return c == '+' || c == '-' || c == '*' || c == '/' ||
           c == '=' || c == '<' || c == '>' || c == '!' ||
           c == ',' || c == '.' || c == ':' || c == ';' ||
           c == '(' || c == ')' || c == '{' || c == '}';
}

LexemeAnalyzer::LexemeAnalyzer(const char *filename) : in(filename), cur_line(1) {}
Lexeme LexemeAnalyzer::get()
{
    enum State {H, Ident, Number, Comment, StrLiteral, Separator} state = H;
    char c;
    std::string buf;
    int number;
    for(;;)
    {
        in.get(c);
        switch(state)
        {
            case H:
                if (in.eof()) return {lex_null, 0};
                else if (isspace(c)) { if (c == '\n') ++cur_line; }
                else if (isalpha(c)) { buf += c; state = Ident; }
                else if (isdigit(c)) { number = c - '0'; state = Number; }
                else if (c == '"') state = StrLiteral;
                else if (c == '/')
                {
                    if (in.peek() == '*') { in.get(); state = Comment; }
                    else { in.unget(); state = Separator; }
                }
                else if (is_separator(c)) { in.unget(); state = Separator; }
                else throw "Unexpected symbol";
                break;
            case Ident:
                if (in.eof() || !isalnum(c))
                {
                    if (!in.eof()) in.unget();
                    int i = find(buf, Lexeme::keywords);
                    if (i == -1)
                    {

                        return {lex_ident, put(buf)};
                    }
                    else return {(LexemeType)i, i};
                }
                else buf += c;
                break;
            case Number:
                if (in.eof() || !isdigit(c))
                {
                    if (!in.eof()) in.unget();
                    return {lex_integer_number, number};
                }
                else number = number * 10 + (c - '0');
                break;
            case Comment:
                if(in.eof()) throw "Never ending comment!!!";
                else if(c == '*' && in.peek() == '/') { in.get(); state = H; }
                break;
            case StrLiteral:
                if(in.eof()) throw "Never ending string literal!!!";
                else if(c != '"') buf += c;
                else
                {
                    str_literals.push_back(buf);
                    return {lex_string_literal, (int)str_literals.size() - 1};
                }
                break;
            case Separator:
                buf += c;
                if(c == '=' || c == '<' || c == '>' || c == '!')
                {
                    if(in.peek() == '=') { in.get(); buf += '='; }
                    else if (c == '!') throw "Unexpected separator: \"!\"";
                }
                int i = find(buf, Lexeme::separators);
                return {(LexemeType)(i + 20), i}; // TODO fix 20 to var
        }
    }
}
