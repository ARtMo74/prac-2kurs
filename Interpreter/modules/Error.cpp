#include "Error.h"

Error::Error(std::string s, int line) : error_msg(std::move(s)), line_(line) {}
Error::Error(const char *s, int line) : error_msg(s), line_(line) {}
const char *Error::what() { return error_msg.c_str(); }
int Error::line() const { return line_; }
