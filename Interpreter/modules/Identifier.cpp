#include <string>
#include <vector>
#include "Identifier.h"

std::vector<Identifier> idents;

Identifier::Identifier(std::string name, LexemeType type)
: name(std::move(name)), type(type), is_defined(false) {}

const std::string &Identifier::get_name()
{
    return name;
}

bool Identifier::operator==(const std::string &s) const
{
    return name == s;
}

int put(const std::string &s)
{
    for (int i = 0; i < idents.size(); ++i)
        if (idents[i] == s)
            return i;
    idents.emplace_back(s);
    return (int)idents.size() - 1;
}
