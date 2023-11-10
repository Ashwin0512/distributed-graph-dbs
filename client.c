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

void clearBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    key_t key;
    int msg_id;
    struct msg_buffer message;

    // Create a unique key for the message queue
    key = ftok("/tmp", MSG_KEY);

    // Get the message queue ID
    msg_id = msgget(key, 0666);
    if (msg_id == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    printf("Client: Connected to Message Queue with key %d\n", key);

    while (1) {
        int option;
        printf("\nOption Menu:\n");
        printf("1. Add a new graph to the database\n");
        printf("2. Modify an existing graph of the database\n");
        printf("3. Perform DFS on an existing graph in the database\n");
        printf("4. Perform BFS on an existing graph in the database\n");
        printf("5. Destroy mqueue and exit.\n");

        scanf("%d", &option);
        clearBuffer();

        int seq_no, op_no;
        char filename[10];

        printf("\nEnter sequence number: ");
        scanf("%d", &seq_no);

        printf("\nEnter Operation Number: ");
        scanf("%d", &op_no);

        printf("\nEnter Graph File Name: ");
        scanf("%s", filename);

        if(op_no == 5) {
            strcpy(message.msg_text, "exit");
        } else if(op_no == 1 || op_no == 2 || op_no == 3 || op_no == 4) {
            snprintf(message.msg_text, MAX_MSG_SIZE, "%d %d %s", seq_no, op_no, filename);
        } else {
            printf("Wrong option chosen\n");
            continue;
        }

        // Set the message type to 1
        message.msg_type = REQUEST_TYPE;

        // Send the message
        if (msgsnd(msg_id, &message, sizeof(message.msg_text), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }

        printf("Client: Message sent to Load Balancer\n");
    }
    return 0;
}