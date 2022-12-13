#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

struct Message
{
    long type;
    char text[256];
};


int main()
{
    key_t key = ftok("/home/artmo", 42);
    int msg_queue = msgget(key, 0666);

    struct Message message;
    message.type = (long) getpid();
    printf("Sender [%ld]\n", message.type);

    while (fgets(message.text, 256, stdin) != NULL)
        msgsnd(msg_queue, &message, strlen(message.text) + 1, 0);

    strcpy(message.text, "done");
    msgsnd(msg_queue, &message, 5, 0);
}
