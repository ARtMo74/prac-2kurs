# Compile a program

gcc -Wall -Werror -g -c -o ./modules/cli.o ./modules/cli.c
gcc -Wall -Werror -g -c -o ./modules/parser.o ./modules/parser.c
gcc -Wall -Werror -g -c -o ./modules/bckg_processes.o ./modules/bckg_processes.c
gcc -Wall -Werror -g -o myshell.out myshell.c ./modules/cli.o ./modules/parser.o ./modules/bckg_processes.o
