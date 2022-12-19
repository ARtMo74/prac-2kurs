#ifndef PRAC_2KURS_BCKG_PROCESSES_H
#define PRAC_2KURS_BCKG_PROCESSES_H

#include "parser.h"

typedef struct pid_list_node
{
    struct pid_list_node *next;
    int pid;
} pid_list_node, *pid_list;

typedef struct process
{
    And_or_list ao_list;
    int pid, status;
} process;

typedef struct bckg_proc_node
{
    struct bckg_proc_node *next;
    process proc;

} bckg_proc_node, *bckg_proc_list;

void push_pid(pid_list *pid_lst, int pid);
int pop_pid(pid_list *pid_lst, int pid);

void push_proc(bckg_proc_list *lst, process proc);
process pop_proc(bckg_proc_list *lst, int pid);

#endif
