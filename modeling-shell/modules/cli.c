/*
getopt reference:
https://www.gnu.org/software/libc/manual/html_node/Getopt.html
*/

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include "cli.h"
#include <fcntl.h>
#include <unistd.h>


void get_options(int argc, char *const *argv)
{
    int i, c, option_index;
    char *filename_r = NULL;
    struct option long_options[] = {
        {"input", required_argument, 0, 'i'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    for (;;)
    {
        c = getopt_long_only(argc, argv, "i:h", long_options, &option_index);
        if (c == -1) break;
        switch(c)
        {
            case 'i':
                filename_r = optarg;
                break;
            case 'h':
                puts(
                    "List of available options:\n"
                    " -h,  --help            | see this help message\n"
                    " -i,  --input           | specify input file"
                );
                exit(0);
            case '?':
                break;
        }
    }

    if (filename_r != NULL)
    {
        int fd = open(filename_r, O_RDONLY, 0666);
        if (fd == -1)
        {
            perror(filename_r);
            exit(1);
        }
        dup2(fd, 0);
        close(fd);
    }

    for (i = optind; i < argc; i++)
        printf("Non-option argument '%s' ignored.\n", argv[i]);
}
