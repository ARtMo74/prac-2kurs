#include <iostream>

class String
{
    char *string;
    int size;
public:
    String() : size(0), string(new char[1]) {}
    String(const String &other)
    {
        string = new char[(size = other.size) + 1];
        for (int i = 0; i <= size; i++) string[i] = other[i];
    }
    explicit String(const char *s)
    {
        for (size = 0; s[size] != '\0'; size++);
        string = new char[size + 1];
        for (int i = 0; i < size; i++) string[i] = s[i];
        string[size] = '\0';
    }
    ~String() {delete[] string;}

    char& operator[](int i) {return string[i];}
    char operator[](int i) const {return string[i];}

    String& operator=(const String &other);
    String& operator=(String &&other) noexcept;
    String& operator+=(const String &other);
    String& operator-=(const String &other);
    String& operator*=(int multiplier);

    friend String operator+(String lhs, const String &rhs) {return lhs += rhs;}
    friend String operator-(String lhs, const String &rhs) {return lhs -= rhs;}
    String operator*(int multiplier) const {return String(*this) *= multiplier;}

    bool operator<(const String &other) const;
    bool operator>(const String &other) const {return other < *this;}
    bool operator>=(const String &other) const {return !(*this < other);}
    bool operator<=(const String &other) const {return !(other < *this);}
    bool operator==(const String &other) const;
    bool operator!=(const String &other) const {return !(*this == other);}

    friend std::ostream& operator<<(std::ostream &os, const String &s);

    int find(const String &other, int start=0, int end=-1);
};

String& String::operator=(const String &other)
{
    if (this == &other) return *this;
    delete[] string;
    string = new char[(size = other.size) + 1];
    for (int i = 0; i <= size; i++) string[i] = other[i];
    return *this;
}
String& String::operator=(String &&other) noexcept
{
    std::swap(size, other.size);
    std::swap(string, other.string);
    return *this;
}
String& String::operator+=(const String &other)
{
    char *tmp = new char[size + other.size + 1];
    for (int i = 0; i < size; i++) tmp[i] = string[i];
    for (int i = size; i <= size + other.size + 1; i++) tmp[i] = other[i];
    delete[] string;
    string = tmp;
    size += other.size;
    return *this;
}
String& String::operator-=(const String &other)
{
    int i = this->find(other);
    if (i == -1) return *this;
    char *tmp = new char[size - other.size + 1];
    for (int j = 0; j < i; j++) tmp[j] = string[j];
    for (int j = i; j < size - other.size; j++)
        tmp[j] = string[j + other.size];
    tmp[size -= other.size] = '\0';
    delete[] string;
    return *this;
}
String& String::operator*=(int multiplier)
{
    char *tmp = new char[size * multiplier + 1];
    for (int j = 0; j < multiplier; j++)
        for (int i = 0; i < size; i++) tmp[j * size + i] = string[i];
    size *= multiplier;
    tmp[size] = '\0';
    delete[] string;
    string = tmp;
    return *this;
}
bool String::operator<(const String &other) const
{
    int i = 0, min_size = std::min(size, other.size);
    while (i < min_size && string[i] == other[i]) i++;
    return i == min_size ? size < other.size : string[i] < other[i];
}
bool String::operator==(const String &other) const
{
    if (size != other.size) return false;
    for (int i = 0; i < size; i++) if (string[i] != other[i]) return false;
    return true;
}
std::ostream& operator<<(std::ostream &os, const String &s)
{
    os << s.string;
    return os;
}

int String::find(const String &other, int start, int end)
{
    if (start != 0 && end == -1) end = start, start = 0;
    else if (end == -1) end = size;
    // TODO maybe add throw here if start or end exceed index range
    for (int i = start; i < end - other.size; i++)
    {
        int j = 0;
        while (i + j < end && j < other.size && string[i + j] == other[j]) j++;
        if (j == other.size) return i;
    }
    return -1;
}


int main()
{
    String s("abc");
}
