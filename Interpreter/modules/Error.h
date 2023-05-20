#ifndef INTERPRETER_ERROR_H
#define INTERPRETER_ERROR_H


#include <exception>
#include <string>


class Error : std::exception
{
    std::string error_msg;
    int line_;
public:
    Error(std::string s, int line);
    Error(const char *s, int line);
    const char *what();
    int line() const;
};


#endif
