#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>

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
    lex_number, lex_string_literal,
    lex_ident,

    lex_null
};

std::vector<std::string> idents;
std::vector<std::string> str_literals;

struct Lexeme
{
    static const char* keywords[];
    static const char* separators[];
    LexemeType type;
    int data;
    explicit Lexeme(LexemeType t=lex_null, int d=0) : type(t), data(d) {}
};

std::ostream& operator<<(std::ostream& stream, Lexeme lex)
{
    stream << lex.type <<": ";
    if (lex.type < 20)
        stream << Lexeme::keywords[lex.data];
    else if (lex.type < 39)
        stream << Lexeme::separators[lex.data];
    else if (lex.type == lex_number)
        stream << lex.data;
    else if (lex.type == lex_string_literal)
        stream << str_literals[lex.data];
    else
        stream << idents[lex.data];
    return stream;
}

const char* keywords[] = {
        "program",
        "int", "real", "string", "boolean",
        "true", "false",
        "if", "else",
        "do", "while", "for",
        "read", "write",
        "goto",
        "and", "or", "not", nullptr
};
const char* separators[] = {
        "+", "-", "*", "/",
        "=",
        "<", ">", "==", "<=", ">=", "!=",
        ",", ".", ":", ";",
        "(", ")", "{", "}", nullptr
};

class LexemeAnalyzer
{
    std::ifstream in;
    static int find(const std::string &s, const char** str_array)
    {
        int i = 0;
        while (str_array[i] != nullptr && std::string(str_array[i]) != s)
            i++;
        if (str_array[i] == nullptr)
            return -1;
        else
            return i;
    }
public:
    explicit LexemeAnalyzer(const char *filename) : in(filename) {}
    Lexeme get_lex()
    {
        enum State {H, Ident, Number, Comment, StrLiteral, Separator} state = H;
        char c;
        std::string buf, str_literal;
        int number;
        do
        {
            in.get(c);
            switch(state)
            {
                case H:
                    if (in.eof()) return Lexeme(lex_null, 0);
                    else if (isalpha(c)) { buf += c; state = Ident; }
                    else if (isdigit(c)) { number = c - '0'; state = Number; }
                    else if (c == '"') state = StrLiteral;
                    else if (c == '/')
                    {
                        if (in.peek() == '*') { in.get(); state = Comment; }
                        else { in.unget(); state = Separator; }
                    }
                    else if (ispunct(c)) { in.unget(); state = Separator; }
                    break;
                case Ident:
                    if (in.eof() || !isalnum(c))
                    {
                        if (!in.eof()) in.unget();
                        int i = find(buf, keywords);
                        if (i == -1)
                        {
                            idents.push_back(buf);
                            return Lexeme(lex_ident, (int)idents.size() - 1);
                        }
                        else return Lexeme((LexemeType)i, i);
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
                    if(in.eof()) throw "Never ending comment!!!";
                    else if(c == '*' && in.peek() == '/') { in.get(); state = H; }
                    break;
                case StrLiteral:
                    if(in.eof()) throw "Never ending string literal!!!";
                    else if(c != '"') str_literal += c;
                    else
                    {
                        str_literals.push_back(str_literal);
                        return Lexeme(lex_string_literal, str_literals.size() - 1);
                    }
                    break;
                case Separator:
                    std::string sep;
                    sep += c;
                    if(c == '=' || c == '<' || c == '>' || c == '!')
                    {
                        if(in.peek() == '=') { in.get(); sep += '='; }
                        else if (c == '!') throw "Unexpected separator: \"!\"";
                    }
                    int i = find(sep, separators);
                    return Lexeme((LexemeType)(i + 14), i);
            }
        }while(true);
    }
};


int main(int argc, const char **argv)
{
    Lexeme lex;
    LexemeAnalyzer lex_analyzer(argv[1]);
    while (true)
    {
        try { lex = lex_analyzer.get_lex(); }
        catch(const char *error_msg)
        {
            std::cout << error_msg << std::endl;
            return 1;
        }
        if(lex.type == lex_null)
            break;
        else
            std::cout << lex << std::endl;
    }
}
