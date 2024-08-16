
#ifndef INFO_H
#define INFO_H

#define QUEUE_NAME  "/service_queue"
#define QUEUE_PERMISSIONS 0660
#define MAX_SIZE 512
#define MAX_NAME_LEN 15
#define QUIT 5
#define CHAT 4
#define MEMBERS 3
#define TEXT 2
#define NAME 1
#define HISTORY 0
#define MAX_CLIENTS 10

typedef struct {
    long type;
    char text[MAX_SIZE];
    char client_name[MAX_NAME_LEN+1];
} message_t;

#endif