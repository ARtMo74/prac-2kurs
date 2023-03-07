#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>

class Notepad
{
    struct Date;
    struct Time;
    struct Person;
    struct Entry;
    static short max_num_entries;
    static short cur_num_entries;
    Entry **entries;
public:
    Notepad() {entries = new Entry*[max_num_entries];}
    ~Notepad();
    enum cmpMode {by_date, by_name};
    void add(char month, char day,
             char start_h, char start_m,
             char end_h, char end_m,
             const char *name, const char *surname,
             const char *phone_number, const char *info);
    void cancel(char month, char day);
    void cancel(char month, char day, char hours, char minutes);
    void cancel(const char *name, const char *surname);
    void sort(cmpMode mode);
    void find(char month, char day);
    void find(char month, char day, char hours, char minutes);
    void find(const char *name, const char *surname);
    void print();
private:
    static bool is_less(const Entry &a, const Entry &b, cmpMode mode);
    void remove(int i);
};

struct Notepad::Date
{
    static const char *MONTHS[];
    static const char *WEEKDAYS[];
    static const short DAYS_PASSED[];
    char m, d;
    explicit Date(char month=0, char day=0) : m(month-1), d(day) {}
    bool operator==(const Date &other) const
    {
        return m == other.m && d == other.d;
    }
    bool operator<(const Date &other) const
    {
        return m < other.m || (m == other.m && d < other.d);
    }
};
struct Notepad::Time
{
    char h, m;
    explicit Time(char hours=-1, char minutes=-1) : h(hours), m(minutes) {}
    bool operator==(const Time &other) const
    {
        return h == other.h && m == other.m;
    }
    bool operator<(const Time &other) const
    {
        return h < other.h || (h == other.h && m < other.m);
    }
};
struct Notepad::Person
{
    char *name, *surname, *phone_number;
    Person(const char* name, const char *surname, const char *phone_number)
    {
        this->name = new char[strlen(name) + 1];
        strcpy(this->name, name);
        this->surname = new char[strlen(surname) + 1];
        strcpy(this->surname, surname);
        this->phone_number = new char[strlen(phone_number) + 1];
        strcpy(this->phone_number, phone_number);
    }
    ~Person() {delete[] name; delete[] surname; delete[] phone_number;}

    bool operator==(const Person &other) const
    {
        return strcmp(name, other.name) == 0
               && strcmp(surname, other.surname) == 0;
    }
    bool operator<(const Person &other) const
    {
        return strcmp(surname, other.surname) < 0
               || strcmp(surname, other.surname) == 0 && strcmp(name, other.name) == 0;
    }
};
struct Notepad::Entry
{
    Date date;
    Time start, end;
    Person person;
    char *info;
    Entry(char month, char day,
          char start_h, char start_m, char end_h, char end_m,
          const char *name, const char *surname, const char *phone_number,
          const char *info)
          : date(month, day), start(start_h, start_m), end(end_h, end_m),
          person(name, surname, phone_number)
    {
        this->info = new char[strlen(info) + 1];
        strcpy(this->info, info);
    }
    ~Entry() {delete[] info;}
    void print() const
    {
        printf("%s, %s %d\n",
               Date::WEEKDAYS[(Date::DAYS_PASSED[date.m] + date.d) % 7],
               Date::MONTHS[date.m],
               date.d);
        printf("Starts at %02d:%02d, ends at %02d:%02d\n",
               start.h, start.m, end.h, end.m);
        printf("%s %s, %s\n", person.name, person.surname, person.phone_number);
        printf("%s\n", info);
    }
};

Notepad::~Notepad()
{
    for (int i = 0; i < cur_num_entries; i++) delete entries[i];
    delete[] entries;
}
void Notepad::add(char month, char day,
                  char start_h, char start_m,
                  char end_h, char end_m,
                  const char *name, const char *surname,
                  const char *phone_number, const char *info)
{
    if (cur_num_entries + 1 == max_num_entries)
    {
        max_num_entries += 5;
        auto **tmp = new Entry*[max_num_entries]; // TODO change back to Entry
        for (int i = 0; i < cur_num_entries; i++) tmp[i] = entries[i];
        delete[] entries;
        entries = tmp;
    }
    entries[cur_num_entries] = new Entry(month, day,
                                         start_h, start_m, end_h, end_m,
                                         name, surname, phone_number,
                                         info);
    cur_num_entries++;
}

void Notepad::cancel(char month, char day)
{
    Date date(month, day);
    for (int i = 0; i < cur_num_entries; i++)
        if (entries[i]->date == date) {remove(i); i--;}
}
void Notepad::cancel(char month, char day, char hours, char minutes)
{
    int i = 0;
    Date date(month, day);
    Time time(hours, minutes);
    while (!(entries[i]->date == date && entries[i]->start == time)) i++;
    remove(i);
}
void Notepad::cancel(const char *name, const char *surname)
{
    Person person(name, surname, "");
    for (int i = 0; i < cur_num_entries; i++)
        if (entries[i]->person == person) {remove(i); i--;}
}
void Notepad::find(char month, char day)
{
    Date date(month, day);
    for (int i = 0; i < cur_num_entries; i++)
        if (entries[i]->date == date) {entries[i]->print(); puts("");}
}
void Notepad::find(const char *name, const char *surname)
{
    Person person(name, surname, "");
    for (int i = 0; i < cur_num_entries; i++)
        if (entries[i]->person == person) {entries[i]->print(); puts("");}
}
void Notepad::find(char month, char day, char hours, char minutes)
{
    int i = 0;
    Date date(month, day);
    Time time(hours, minutes);
    while (!(entries[i]->date == date && entries[i]->start == time)) i++;
    entries[i]->print(); puts("");
}
bool Notepad::is_less(const Entry &a, const Entry &b, cmpMode mode)
{
    if (mode == by_date) {return a.date < b.date || a.date == b.date && a.start < b.start;}
    else {return a.person < b.person;}
}
void Notepad::sort(cmpMode mode)
{
    for (int i = 0; i < cur_num_entries - 1; i++)
        for (int j = i + 1; j < cur_num_entries; j++)
        {
            if (is_less(*entries[j], *entries[i], mode))
            {
                Entry *tmp = entries[i];
                entries[i] = entries[j];
                entries[j] = tmp;
            }
        }
}
void Notepad::print()
{
    for (int i = 0; i < cur_num_entries; i++) {entries[i]->print(); puts("");}
}
void Notepad::remove(int i)
{
    puts("Canceled:"); entries[i]->print(); puts("");
    delete entries[i];
    cur_num_entries--;
    for (int j = i; j < cur_num_entries; j++)
        entries[j] = entries[j + 1];
}


const char *Notepad::Date::MONTHS[] =
        {"January", "February", "March", "April",
         "May", "June", "July", "August",
         "September", "October", "November", "December"};
const char *Notepad::Date::WEEKDAYS[] =
        {"Saturday", "Sunday", "Monday", "Tuesday", "Wednesday",
         "Thursday", "Friday"};
const short Notepad::Date::DAYS_PASSED[] =
        {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 335};
short Notepad::max_num_entries = 5;
short Notepad::cur_num_entries = 0;


int main()
{
    if (fork() == 0) execlp("clear", "clear", nullptr);
    wait(nullptr);
    Notepad notepad;
#define TEST
#ifdef TEST
    notepad.add(3, 6, 20, 15, 23, 45, "Petr", "Petrov", "9876450", "info");
    notepad.add(3, 6, 10, 0, 22, 30, "Ivan", "Ivanov", "1234567", "entry");
    notepad.add(4, 25, 0, 0, 23, 59, "Artem", "Moor", "89600638075", "Happy Birthday to me!!!");
    notepad.add(3, 7, 14, 35, 17, 50, "Igor'", "Golovin", "1234567890", "Prac");
    notepad.add(10, 12, 20, 30, 22, 00, "Artem", "Moor", "89600638075", "abc");
    notepad.print();
    notepad.sort(Notepad::by_date);
    puts("----------------------------------------------------------");
    notepad.print();
    notepad.sort(Notepad::by_name);
    puts("----------------------------------------------------------");
    notepad.print();
    puts("----------------------------------------------------------");
    notepad.find(3, 6);
    puts("----------------------------------------------------------");
    notepad.find("Artem", "Moor");
    puts("----------------------------------------------------------");
    notepad.find(3, 7, 14, 35);
    puts("----------------------------------------------------------");
    notepad.cancel(3, 6);
    puts("----------------------------------------------------------");
    notepad.cancel("Artem", "Moor");
    puts("----------------------------------------------------------");
    notepad.cancel(3, 7, 14, 35);
    puts("----------------------------------------------------------");
    notepad.print();
    puts("----------------------------------------------------------");
    notepad.add(1, 2, 3, 4, 5, 6, "abc", "def", "1232444", "qwerty");
    notepad.print();
#else
    char command[16];
    while (true)
    {
        scanf("%s", command);
        if (strcmp(command, "exit") == 0) break;
        else if (strcmp(command, "clear") == 0)
        {
            if(fork() == 0) execlp("clear", "clear", nullptr);
            wait(nullptr);
        }
        else if (strcmp(command, "add") == 0)
        {
            char name[10], surname[10], phone_number[15], info[100];
            char day, month, start_h, start_m, end_h, end_m;
            printf("Full name and phone number: ");
            scanf("%s %s %s", name, surname, phone_number);
            printf("Date and time: ");
            scanf("\n%hhd.%hhd %hhd:%hhd - %hhd:%hhd",
                  &month, &day, &start_h, &start_m, &end_h, &end_m);
            notepad.add(month, day, start_h, start_m, end_h, end_m,
                        name, surname, phone_number, info);
        }
        else if (strcmp(command, "cancel") == 0)
        {
            char c;
            scanf(" -%c", &c);
            if (c == 'p')
            {
                char name[16], surname[16];
                scanf("%s %s", name, surname);
                notepad.cancel(name, surname);
            }
            else if (c == 'd')
            {
                char month, day, hours, minutes;
                if (scanf("%c.%c %c:%c", &month, &day, &hours, &minutes) == 2)
                    notepad.cancel(month, day);
                else notepad.cancel(month, day, hours, minutes);
            }
            else printf("Unknown specification: %c\n", c);
        }
        else if (strcmp(command, "find") == 0)
        {
            char c;
            scanf(" -%c", &c);
            if (c == 'p')
            {
                char name[16], surname[16];
                scanf("%s %s", name, surname);
                notepad.find(name, surname);
            }
            else if (c == 'd')
            {
                char month, day, hours, minutes;
                if (scanf("%c.%c %c:%c", &month, &day, &hours, &minutes) == 2)
                    notepad.find(month, day);
                else notepad.find(month, day, hours, minutes);
            }
            else printf("Unknown specification: %c\n", c);
        }
        else if (strcmp(command, "sort") == 0)
        {
            char mode[5];
            scanf(" by %s", mode);
            if (strcmp(mode, "date") == 0) notepad.sort(Notepad::by_date);
            else notepad.sort(Notepad::by_name);

        }
        else if (strcmp(command, "print") == 0) notepad.print();
        else printf("Unknown command\n");
    }
#endif
}
