#include <iostream>
#include <vector>
#include "modules/SyntaxAnalyzer.h"

std::vector<std::string> str_literals;


int main(int argc, const char **argv)
{
    SyntaxAnalyzer syntax_analyzer(argv[1]);
    try { syntax_analyzer.analyze(); }
    catch(const char *error_msg)
    {
        std::cout << error_msg << std::endl;
        return 1;
    }
}
