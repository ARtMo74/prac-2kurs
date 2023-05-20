#include <iostream>
#include <vector>
#include "modules/Parser.h"
#include "modules/Error.h"
std::vector<std::string> str_literals;


int main(int argc, const char **argv)
{
    Parser syntax_analyzer(argv[1]);
    try { syntax_analyzer.analyze(); }
    catch(Error &error)
    {
        std::cout << "line " << error.line() << ": " << error.what() << std::endl;
    }
}
