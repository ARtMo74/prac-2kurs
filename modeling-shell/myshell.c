#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// #include "cli.c"
// #include "parser.c"


typedef struct Pipe_elem
{
    words_array words;
    struct Pipe_elem *next;
    int fd[2];
} Pipe_elem, *Pipeline;


typedef struct pid_list_elem
{
    struct pid_list_elem *next;
    int pid;
} pid_list_elem, *pid_list;


typedef struct process_list_elem
{
    struct process_list_elem *next;
    pid_list pid_lst;
    words_array words;
} process_list_elem, *process_list;


void sigint_handler(int sig)
{
    printf("\n");
}



Pipeline make_pipeline(words_array words)
{
    Pipeline pipeline = NULL, *tmp = &pipeline;
    int i = 0;
    for(;;)
    {
        int n = 0;
        words_array wa = (words_array)malloc(sizeof(str));
        *tmp = (Pipeline)malloc(sizeof(Pipe_elem));
        (*tmp)->fd[0] = (*tmp)->fd[1] = -1;
        while (words[i] != NULL && strcmp(words[i], "|") != 0)
        {
            if (strcmp(words[i], ">") == 0)
            {
                int out = open(words[i + 1], O_CREAT|O_WRONLY|O_TRUNC, 0666);
                (*tmp)->fd[1] = out;
                i += 2;
            }
            else if (strcmp(words[i], "<") == 0)
            {
                int in = open(words[i + 1], O_RDONLY, 0666);
                (*tmp)->fd[0] = in;
                i += 2;
            }
            else if (strcmp(words[i], "&") == 0) i++;
            else
            {
                n++;
                wa = (words_array)realloc(wa, (n + 1) * sizeof(str));
                wa[n - 1] = words[i];
                i++;
            }
        }
        wa[n] = NULL;
        (*tmp)->words = wa;
        (*tmp)->next = NULL;
        tmp = &((*tmp)->next);

        if (words[i] == NULL) break;
        i++;
    }
    return pipeline;
}


void free_words(words_array words)
{
    int i;
    for (i = 0; words[i] != NULL; i++)
        free(words[i]);
    free(words);
}

void free_pipeline(Pipeline pipeline)
{
    while (pipeline != NULL)
    {
        Pipeline tmp = pipeline;
        pipeline = pipeline->next;
        free(tmp->words);
        free(tmp);
    }
}


void push_pid(pid_list *pl, int pid)
{
    pid_list tmp = (pid_list)malloc(sizeof(pid_list_elem));
    tmp->next = *pl;
    tmp->pid = pid;
    *pl = tmp;
}


int pop_pid(pid_list *pl, int pid)
{
    pid_list *tmp = pl;
    while (*tmp != NULL)
    {
        if ((*tmp)->pid == pid)
        {
            pid_list tmp2 = *tmp;
            *tmp = (*tmp)->next;
            free(tmp2);
            return 1;
        }
        else tmp = &((*tmp)->next);
    }
    return 0;
}


int set_background_flag(words_array words)
{
    int i;
    for (i = 0; words[i] != NULL && strcmp(words[i], "&") != 0; i++);
    if (words[i] == NULL) return 0;
    else return words[i + 1] == NULL ? 1: -1;
}


int main(int argc, const str *argv)
{
    FILE *input;
    str line;
    process_list bckg = NULL, done = NULL;
    get_options(argc, argv, &input);
    signal(SIGINT, sigint_handler);

    while ((line = get_line(input)) != NULL)
    {
        int background_flag, pid;
        words_array words = parse_line(line);
        free(line);
        background_flag = set_background_flag(words);
        if (background_flag == -1)
        {
            fputs("'&' is not at the end of the line\n", stderr);
            free_words(words);
            continue;
        }

        Pipeline pipeline = make_pipeline(words);
        if (pipeline->words[0] != NULL)
        {
            if (strcmp(pipeline->words[0], "exit") == 0)
            {
                free_pipeline(pipeline);
                free_words(words);
                break;
            }
            else if (strcmp(pipeline->words[0], "cd") == 0)
            {
                str path;
                if (pipeline->words[1] == NULL) path = getenv("HOME");
                else path = pipeline->words[1];
                if (chdir(path) == -1) perror("chdir failed");
                free_words(words);
            }
            else
            {
                Pipeline tmp = pipeline;
                pid_list cur_pids = NULL;
                int fd_in_backup = dup(0), fd[2];
                while (tmp != NULL)
                {
                    pipe(fd);
                    pid = fork();
                    if (pid == -1)
                    {
                        perror("fork");
                        free_pipeline(pipeline);
                        free(words);
                        return 1;
                    }
                    else if (pid == 0)
                    {
                        if (tmp->next != NULL) dup2(fd[1], 1);
                        close(fd[0]); close(fd[1]);
                        if (tmp->fd[0] != -1)
                        {
                            dup2(tmp->fd[0], 0);
                            close(tmp->fd[0]);
                        }
                        if (tmp->fd[1] != -1)
                        {
                            dup2(tmp->fd[1], 1);
                            close(tmp->fd[1]);
                        }
                        signal(SIGINT, SIG_DFL);
                        execvp(tmp->words[0], tmp->words);
                        perror("execvp");
                        free_pipeline(pipeline);
                        free_words(words);
                        return 2;
                    }
                    dup2(fd[0], 0);
                    close(fd[0]); close(fd[1]);
                    push_pid(&cur_pids, pid);
                    tmp = tmp->next;
                }
                if (background_flag)
                {
                    process_list tmp;
                    tmp = (process_list)malloc(sizeof(process_list_elem));
                    tmp->next = bckg;
                    tmp->pid_lst = NULL;
                    tmp->words = words;
                    while (cur_pids != NULL)
                    {
                        push_pid(&(tmp->pid_lst), cur_pids->pid);
                        pop_pid(&cur_pids, cur_pids->pid);
                    }
                    bckg = tmp;
                }
                else
                {
                    while (cur_pids != NULL)
                    {
                        pid = wait(NULL);
                        if (!pop_pid(&cur_pids, pid))
                        {
                            process_list *tmp = &bckg;
                            while (!pop_pid(&(*tmp)->pid_lst, pid))
                                tmp = &(*tmp)->next;
                            if ((*tmp)->pid_lst == NULL)
                            {
                                process_list tmp2 = *tmp;
                                *tmp = (*tmp)->next;
                                tmp2->next = done;
                                done = tmp2;
                            }
                        }
                    }
                    free_words(words);
                }
                dup2(fd_in_backup, 0);
                close(fd_in_backup);
            }
        }
        else free(words);
        while ((pid = waitpid(-1, NULL, WNOHANG)) != 0 && pid != -1)
        {
            process_list *tmp = &bckg;
            while (!pop_pid(&(*tmp)->pid_lst, pid))
                tmp = &(*tmp)->next;
            if ((*tmp)->pid_lst == NULL)
            {
                process_list tmp2 = *tmp;
                *tmp = (*tmp)->next;
                tmp2->next = done;
                done = tmp2;
            }
        }
        while (done != NULL)
        {
            process_list tmp = done; int i;
            printf("done: ");
            for (i = 0; tmp->words[i] != NULL; i++)
                printf(
                    "%s%s",
                    strcmp(tmp->words[i], "&") != 0 ? tmp->words[i]: "",
                    tmp->words[i + 1] == NULL ? "\n": " "
                );
            done = done->next;
            free_words(tmp->words);
            free(tmp);
        }
        free_pipeline(pipeline);
    }
}
