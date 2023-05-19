#ifndef INTERPRETER_LEXEME_ANALYZER_H
#define INTERPRETER_LEXEME_ANALYZER_H


#include <fstream>
#include "Lexeme.h"

class LexemeAnalyzer
{
    std::ifstream in;
    int cur_line;
    static int find(const std::string &s, const char** str_array);
    static bool is_separator(char c);
public:
    explicit LexemeAnalyzer(const char *filename);
    Lexeme get();
};


#endif
