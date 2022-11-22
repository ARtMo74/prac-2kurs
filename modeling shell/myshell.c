#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cli.c"
#include "parser.c"


typedef struct Pipeline
{
    words_array words;
    struct Pipeline *next;
    int fd[2];
} Pipeline;


Pipeline *make_pipeline(words_array words)
{
    Pipeline *pipeline = NULL, **tmp = &pipeline;
    int i = 0;
    for(;;)
    {
        int n = 0;
        words_array wa = (words_array)malloc(sizeof(str));
        *tmp = (Pipeline*)malloc(sizeof(Pipeline));
        (*tmp)->fd[0] = (*tmp)->fd[1] = -1;
        while (words[i] != NULL && strcmp(words[i], "|") != 0)
        {
            if (strcmp(words[i], ">") == 0)
            {
                int out = open(words[i + 1], O_CREAT|O_WRONLY|O_TRUNC, 0666);
                (*tmp)->fd[1] = out;
                free(words[i]); free(words[i + 1]);
                i += 2;
            }
            else if (strcmp(words[i], "<") == 0)
            {
                int in = open(words[i + 1], O_RDONLY, 0666);
                (*tmp)->fd[0] = in;
                free(words[i]); free(words[i + 1]);
                i += 2;
            }
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
        free(words[i]);
        i++;
    }
    free(words);
    return pipeline;
}


void free_pipeline(Pipeline *pipeline)
{
    while (pipeline != NULL)
    {
        Pipeline *tmp = pipeline;
        pipeline = pipeline->next;
        int i;
        for (i = 0; tmp->words[i] != NULL; i++)
            free(tmp->words[i]);
        free(tmp->words);
        free(tmp);
    }
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
    FILE *input, *output;
    str line;
    process_args(argc, argv, &input, &output);

    while ((line = get_line(input)) != NULL)
    {
        int background_flag;
        words_array words = parse_line(line);
        free(line);
        background_flag = set_background_flag(words);
        if (background_flag == -1)
        {
            fputs("'&' is not at the end of the line\n", stderr);
            int i;
            for (i = 0; words[i] != NULL; i++)
                free(words[i]);
            free(words);
            continue;
        }

        Pipeline *pipeline = make_pipeline(words);
        if (pipeline->words[0] != NULL)
        {
            if (strcmp(pipeline->words[0], "exit") == 0)
            {
                free_pipeline(pipeline);
                break;
            }
            else if (strcmp(pipeline->words[0], "cd") == 0)
            {
                str path;
                if (pipeline->words[1] == NULL) path = getenv("HOME");
                else path = pipeline->words[1];
                if (chdir(path) == -1) perror("chdir failed");
            }
            else
            {
                Pipeline *tmp = pipeline;
                int fd_in_backup = dup(0), fd[2], pid;
                while (tmp != NULL)
                {
                    pipe(fd);
                    pid = fork();
                    if (pid == -1)
                    {
                        perror("fork");
                        free_pipeline(pipeline);
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
                        execvp(tmp->words[0], tmp->words);
                        perror("execvp");
                        free_pipeline(pipeline);
                        return 2;
                    }
                    dup2(fd[0], 0);
                    close(fd[0]); close(fd[1]);
                    tmp = tmp->next;
                }
                while(wait(NULL) != -1);
                dup2(fd_in_backup, 0);
                close(fd_in_backup);
            }
        }
        free_pipeline(pipeline);
    }
}
