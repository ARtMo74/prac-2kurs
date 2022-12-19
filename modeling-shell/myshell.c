#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "modules/parser.h"
#include "modules/cli.h"
#include "modules/bckg_processes.h"


static bckg_proc_list running = NULL, finished = NULL;


void invite()
{
    char *username = getenv("USER");
    char *home = getenv("HOME");
    char *cwd = getcwd(NULL, 0);
    char *path = cwd;
    if (strncmp(cwd, home, strlen(home)) == 0) path += strlen(home);
    char hostname[65];
    gethostname(hostname, sizeof(hostname));
    printf("%s@%s:", username, hostname);
    if (path == cwd) printf("%s$ ", cwd);
    else printf("~%s$ ", path);
    fflush(stdout);
    free(cwd);
}


void handler1(int sig)
{
    puts("");
    invite();
    fflush(stdout);
}


void handler2(int sig)
{
    puts("");
}


int logic_check(int status, enum logic_operator opr)
{
    return (status == 0 && opr == and_opr) || (status != 0 && opr == or_opr);
}


int run_pipeline(Pipeline pl, int in_bckg)
{
    pid_list cur_pids = NULL;
    int fd0_backup = dup(0), fd[2], exit_status, pid;
    while (pl != NULL)
    {
        pipe(fd);
        pid = fork();
        if (pid == 0) {
            if (pl->next != NULL) dup2(fd[1], 1);
            close(fd[0]);
            close(fd[1]);
            if (pl->cmd.fd[0] != -1) {
                dup2(pl->cmd.fd[0], 0);
                close(pl->cmd.fd[0]);
            }
            if (pl->cmd.fd[1] != -1) {
                dup2(pl->cmd.fd[1], 1);
                close(pl->cmd.fd[1]);
            }
            if (!in_bckg) signal(SIGINT, SIG_DFL);
            execvp(pl->cmd.argv[0], pl->cmd.argv);
            perror("execvp");
            exit(2);
        }
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);
        push_pid(&cur_pids, pid);
        pl = pl->next;
    }
    dup2(fd0_backup, 0);
    close(fd0_backup);

    while (cur_pids != NULL)
    {
        int status, done;
        done = wait(&status);
        if (!pop_pid(&cur_pids, done))
        {
            process proc = pop_proc(&running, done);
            proc.status = status;
            push_proc(&finished, proc);
        }
        else if (done == pid) exit_status = status;
    }
    return exit_status;
}


int run_ao_list(And_or_list ao_list, int in_bckg)
{
    int exit_status;
    signal(SIGINT, handler2);
    while (ao_list != NULL)
    {
        exit_status = run_pipeline(ao_list->pl, in_bckg);
        if (logic_check(exit_status, ao_list->opr))
            ao_list = ao_list->next;
        else break;
    }
    signal(SIGINT, handler1);
    return exit_status;
}


void print_ao_list(And_or_list ao_list)
{
    while (1)
    {
        Pipeline pl = ao_list->pl;
        while (1)
        {
            int i; Simple_command cmd = pl->cmd;
            for (i = 0; cmd.argv[i] != NULL; i++)
                printf("%s%s", cmd.argv[i], cmd.argv[i + 1] != NULL ? " " : "");
            pl = pl->next;
            if (pl == NULL) break;
            printf(" | ");
        }
        if (ao_list->next == NULL) { puts(""); break; }
        printf(" %s ", ao_list->opr == and_opr ? "&&" : "||");
        ao_list = ao_list->next;
    }
}


void check_finished()
{
    int status, pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        process proc = pop_proc(&running, pid);
        proc.status = status;
        push_proc(&finished, proc);
    }
    while (finished != NULL)
    {
        process proc = pop_proc(&finished, finished->proc.pid);
        if (WIFEXITED(proc.status))
        {
            if (WEXITSTATUS(proc.status) == 0)
                printf("%-30s", "Done");
            else
                printf("Exit %-25d", WEXITSTATUS(proc.status));
        }
        else printf("%-30s", "Killed");
        print_ao_list(proc.ao_list);
    }
}


int main(int argc, const str *argv)
{
    List command;
    get_options(argc, argv);
    signal(SIGINT, handler1);
    invite();

    while ((command = make_command()) != NULL)
    {
        words_array cmd_argv = command->ao_list->pl->cmd.argv;
        if (cmd_argv[0] != NULL)
        {
            if (strcmp(cmd_argv[0], "exit") == 0)
            {
                // free all
                break;
            }
            else if (strcmp(cmd_argv[0], "cd") == 0)
            {
                str path;
                if (cmd_argv[1] == NULL) path = getenv("HOME");
                else path = cmd_argv[1];
                if (chdir(path) == -1) perror("chdir failed");
                // add free(cmd)
            }
            else
            {
                List tmp = command;
                while (tmp != NULL)
                {
                    if (tmp->is_in_bckg)
                    {
                        int pid = fork();
                        if (pid == 0)
                        {
                            int exit_status = run_ao_list(tmp->ao_list, 1);
                            exit(exit_status);
                        }
                        else
                        {
                            printf("[%d]\n", pid);
                            process proc;
                            proc.pid = pid;
                            proc.ao_list = tmp->ao_list;
                            push_proc(&running, proc);
                        }
                    }
                    else run_ao_list(tmp->ao_list, 0);
                    check_finished();
                    tmp = tmp->next;
                }
            }
        }
        check_finished();
        invite();
    }
}
