#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "cli.c"
#include "parser.c"


void free_all(words_array words, str command)
{
    int i;
    for (i = 0; words[i] != NULL; i++)
        free(words[i]);
    free(words);
    free(command);
}


int main(int argc, const str *argv)
{
    FILE *input, *output;
    words_array words;
    str command;
    process_args(argc, argv, &input, &output);

    while ((command = get_command(input)) != NULL)
    {
        words = parse_command(command);
        if (words[0] != NULL)
        {
            if (strcmp(words[0], "exit") == 0)
            {
                free_all(words, command);
                break;
            }
            else if (strcmp(words[0], "cd") == 0)
            {
                str path = (words[1] == NULL ? getenv("HOME"): words[1]);
                if (chdir(path) == -1) perror("chdir failed");
            }
            else
            {
                int pid = fork();
                if (pid == -1)
                {
                    perror("fork failed");
                    free_all(words, command);
                    return 1;
                }
                if (pid == 0)
                {
                    execvp(words[0], words);
                    perror("execvp failed");
                    free_all(words, command);
                    return 2;
                }
                else wait(NULL);
            }
        }
        free_all(words, command);
    }
}
