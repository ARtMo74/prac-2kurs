#ifndef INTERPRETER_IDENTIFIER_H
#define INTERPRETER_IDENTIFIER_H


#include <string>
#include <vector>
#include "Lexeme.h"

class Identifier {
    std::string name;
public:
    LexemeType type;
    bool is_defined;
    explicit Identifier(std::string name, LexemeType type=lex_null);
    const std::string &get_name();
    bool operator==(const std::string &s) const;
};

extern std::vector<Identifier> idents;

int put(const std::string &s);
#endif
