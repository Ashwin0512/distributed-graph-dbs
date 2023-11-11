#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define SECONDARY_SERVER_1_MSG_TYPE 4

#define MAX_MSG_SIZE 256
#define MSG_KEY 1234

struct msg_buffer {
    long msg_type;
    char msg_text[MAX_MSG_SIZE];
};

int main() {
    key_t key;
    int msg_id;
    struct msg_buffer message;

    key = ftok("/tmp", MSG_KEY);
    if(key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    msg_id = msgget(key, 0666);
    if (msg_id == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    printf("Secondary Server 1: Connected to Message Queue with key %d\n", key);

    while(1) {
        if(msgrcv(msg_id, &message, sizeof(message.msg_text), SECONDARY_SERVER_1_MSG_TYPE, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }

        printf("Secondary Server 1: Received message: %s\n", message.msg_text);
    }

    printf("Secondary Server 1: Exiting\n");
    return 0;
}