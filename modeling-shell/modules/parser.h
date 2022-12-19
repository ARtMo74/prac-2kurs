#ifndef PRAC_MYSHELL_PARSER_H
#define PRAC_MYSHELL_PARSER_H

#include <stdio.h>

typedef char **words_array, *str;

typedef struct Simple_command
{
    words_array argv;
    int fd[2];
} Simple_command;

typedef struct Pipeline_node
{
    struct Pipeline_node *next;
    Simple_command cmd;
} Pipeline_node, *Pipeline;

enum logic_operator { and_opr, or_opr, null_opr };
typedef struct And_or_list_node
{
    struct And_or_list_node *next;
    Pipeline pl;
    enum logic_operator opr;
} And_or_list_node, *And_or_list;

typedef struct List_node
{
    struct List_node *next;
    int is_in_bckg;
    And_or_list ao_list;
} List_node, *List;

List make_command();

#endif
