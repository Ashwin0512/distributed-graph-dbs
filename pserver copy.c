#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <pthread.h>

#define PRIMARY_SERVER_MSG_TYPE 3

#define MAX_MSG_SIZE 256
#define MSG_KEY 1234
#define SHM_KEY 5678

struct msg_buffer {
    long msg_type;
    char msg_text[MAX_MSG_SIZE];
};

struct GraphData {
    int nodes;
    int adj[31][31];
};

void *handleWriteRequest(void *arg) {
    struct msg_buffer *request = (struct msg_buffer *)arg;

    int seq_no, op_no;
    char filename[10];
    sscanf(request->msg_text, "%d %d %s", &seq_no, &op_no, filename);

    key_t shmkey = ftok("/tmp", SHM_KEY);

    int shmid = shmget(shmkey, sizeof(struct GraphData), 0666);
    if(shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach shared memory segment
    struct GraphData *graphData = (struct GraphData *)shmat(shmid, NULL, 0);
    if(graphData == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    if(op_no == 1) {
        printf("Thread: Creating a new graph file: %s\n", filename);
        printf("Printing Graph Data\n");
        printf("Nodes : %d\n", graphData->nodes);
        printf("Adjacency Matrix: \n");
        for(int i=1; i<=graphData->nodes; i++)  {
            for(int j=1; j<=graphData->nodes; j++) {
                printf("%d ", graphData->adj[i][j]);
            }
            printf("\n");
        }
        // Logic ahead
    } else if(op_no == 2) {
        printf("Thread: Modifying an existing graph file: %s\n", filename);
        // Logic ahead
    } else {
        printf("Thread: Unknown Operation: %d\n", op_no);
    }

    // Detach shared memory segment
    if(shmdt(graphData) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    free(request);
    pthread_exit(NULL);
}

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

    printf("Primary Server: Connected to Message Queue with key %d\n", key);

    while(1) {
        if(msgrcv(msg_id, &message, sizeof(message.msg_text), PRIMARY_SERVER_MSG_TYPE, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }

        printf("Primary Server: Received Message: %s\n", message.msg_text);

        // Create a new thread to handle the write request
        pthread_t tid;
        struct msg_buffer *request = malloc(sizeof(struct msg_buffer));
        if(request == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        // Copy the received message to the dynamically allocated structure
        memcpy(request, &message, sizeof(struct msg_buffer));

        if(pthread_create(&tid, NULL, handleWriteRequest, (void *)request)) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }

        // Detach the thread to allow it to run independently
        pthread_detach(tid);
    }

    printf("Primary Server: Exiting\n");
    return 0;
}