#include <vector>
#include <cctype>
#include "Scanner.h"
#include "Identifier.h"
#include "Error.h"

extern std::vector<std::string> str_literals;

int Scanner::find(const std::string &s, const char** str_array)
{
    int i = 0;
    while (str_array[i] != nullptr && std::string(str_array[i]) != s)
        i++;
    if (str_array[i] == nullptr)
        return -1;
    else
        return i;
}

bool Scanner::is_separator(char c)
{
    return c == '+' || c == '-' || c == '*' || c == '/' ||
           c == '=' || c == '<' || c == '>' || c == '!' ||
           c == ',' || c == ':' || c == ';' ||
           c == '(' || c == ')' || c == '{' || c == '}';
}
int Scanner::line() const { return cur_line; }
Scanner::Scanner(const char *filename) : in(filename), cur_line(1) {}
Lexeme Scanner::get()
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
                if (in.eof()) return Lexeme();
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
                else throw Error(std::string("Unexpected symbol: ") + c, cur_line);
                break;
            case Ident:
                if (in.eof() || !isalnum(c))
                {
                    if (!in.eof()) in.unget();
                    int i = find(buf, Lexeme::keywords);
                    if (i == -1) return Lexeme(lex_ident, put(buf));
                    else return Lexeme((LexemeType) i, i);
                }
                else buf += c;
                break;
            case Number:
                if (in.eof() || !isdigit(c))
                {
                    if (!in.eof()) in.unget();
                    return Lexeme(lex_number, number);
                }
                else number = number * 10 + (c - '0');
                break;
            case Comment:
                if(in.eof())
                    throw Error("Unterminated /* comment.", cur_line);
                else if (c == '\n') ++cur_line;
                else if(c == '*' && in.peek() == '/') { in.get(); state = H; }
                break;

            case StrLiteral:
                if(in.eof() || c == '\n')
                    throw Error("Never ending string literal", cur_line);
                else if(c != '"') buf += c;
                else
                {
                    str_literals.push_back(buf);
                    return Lexeme(lex_string_literal, (int)str_literals.size() - 1);
                }
                break;
            case Separator:
                buf += c;
                if(c == '=' || c == '<' || c == '>' || c == '!')
                {
                    if(in.peek() == '=')
                    {
                        in.get();
                        buf += '=';
                    }
                    else if (c == '!')
                        throw Error("! is not defined operation, use 'not' instead", cur_line);
                }
                int i = find(buf, Lexeme::separators);
                return Lexeme((LexemeType)(i + 19), i); // TODO fix 20 to var
        }
    }
}
