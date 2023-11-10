#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define REQUEST_TYPE 1
#define RESPONSE_TYPE 2

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

    // Create a unique key for the message queue
    key = ftok("/tmp", MSG_KEY);
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Create a message queue
    msg_id = msgget(key, 0666 | IPC_CREAT);
    if (msg_id == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    printf("Load Balancer: Message Queue created with key %d\n", key);

    // Infinite loop to receive messages
    while (1) {
        // Receive the message
        if (msgrcv(msg_id, &message, sizeof(message.msg_text), REQUEST_TYPE, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }

        // Print the received message
        printf("Load Balancer: Received message: %s\n", message.msg_text);

        if (strcmp(message.msg_text, "exit") == 0) {
            break;
        }
    }

    // Close and remove the message queue when done
    if (msgctl(msg_id, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }

    printf("Load Balancer: Message Queue destroyed\n");

    return 0;
}
