#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <string.h>

struct Message
{
    long type;
    char text[256];
};


int main(int argc, const char **argv)
{
    key_t key = ftok("/home/artmo", 42);
    int msg_queue = msgget(key, 0666 | IPC_CREAT);
    int n, done = 0;
    struct Message message;
    sscanf(argv[1], "%d", &n);
    for (;;)
    {
        msgrcv(msg_queue, &message, 256, 0, 0);
        if (strcmp(message.text, "done") == 0)
        {
            printf("[%ld] is done.\n", message.type);
            done += 1;
            if (done == n)
            {
                puts("Done.");
                msgctl(msg_queue, IPC_RMID, NULL);
                break;
            }
        }
        else printf("[%ld]: %s", message.type, message.text);
    }
}
