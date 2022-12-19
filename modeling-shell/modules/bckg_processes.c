#include <stdlib.h>
#include "bckg_processes.h"


void push_pid(pid_list *pid_lst, int pid)
{
    pid_list tmp = (pid_list)malloc(sizeof(pid_list_node));
    tmp->next = *pid_lst;
    tmp->pid = pid;
    *pid_lst = tmp;
}


int pop_pid(pid_list *pid_lst, int pid)
{
    pid_list *tmp = pid_lst;
    while (*tmp != NULL)
    {
        if ((*tmp)->pid == pid)
        {
            pid_list tmp2 = *tmp;
            *tmp = (*tmp)->next;
            free(tmp2);
            return 1;
        }
        else tmp = &(*tmp)->next;
    }
    return 0;
}


void push_proc(bckg_proc_list *lst, process proc)
{
    bckg_proc_list tmp = (bckg_proc_list)malloc(sizeof(bckg_proc_node));
    tmp->next = *lst;
    tmp->proc = proc;
    *lst = tmp;
}


process pop_proc(bckg_proc_list *lst, int pid)
{
    bckg_proc_list *tmp = lst;
    while ((*tmp)->proc.pid != pid)
        tmp = &(*tmp)->next;

    bckg_proc_list tmp2 = *tmp;
    process proc = (*tmp)->proc;
    *tmp = (*tmp)->next;
    free(tmp2);
    return proc;
}
