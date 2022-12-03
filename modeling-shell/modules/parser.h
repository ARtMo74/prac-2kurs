#ifndef PRAC_MYSHELL_PARSER_H
#define PRAC_MYSHELL_PARSER_H

typedef char **words_array, *str;

typedef struct Simple_command
{
    words_array argv;
    int fd[2];
} Simple_command;

typedef struct Pipeline
{
    struct Pipeline *next;
    Simple_command cmd;
} Pipeline;

enum ctrl_operator { and_opr, or_opr, null_opr };
typedef struct And_or_list
{
    struct And_or_list *next;
    Pipeline pl;
    ctrl_operator opr;
} And_or_list;

typedef struct List
{
    struct List *next;
    int is_in_bckg;
    And_or_list and_or_list;
} List;

#endif
