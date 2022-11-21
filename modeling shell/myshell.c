#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "cli.c"
#include "parser.c"


void free_all(words_array words, str line)
{
    int i;
    for (i = 0; words[i] != NULL; i++)
        free(words[i]);
    free(words);
    free(line);
}


int main(int argc, const str *argv)
{
    FILE *input, *output;
    str line;
    process_args(argc, argv, &input, &output);

    while ((line = get_line(input)) != NULL)
    {
        words_array words = parse_line(line);
        if (words[0] != NULL)
        {
            if (strcmp(words[0], "exit") == 0)
            {
                free_all(words, line);
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
                    free_all(words, line);
                    return 1;
                }
                if (pid == 0)
                {
                    execvp(words[0], words);
                    perror("execvp failed");
                    free_all(words, line);
                    return 2;
                }
                else wait(NULL);
            }
        }
        free_all(words, line);
    }
}
