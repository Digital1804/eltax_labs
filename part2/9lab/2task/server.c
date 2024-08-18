#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>

#include "info.h"

sem_t *sem_msg, *sem_members, *sem_chat;
members_t *members_ptr;
message_t *msg_ptr;
chat_t *chat_ptr;
bool running = true;

/** @brief Обработчик сигнала
    При сочетании клавиш <Ctrl+C> вызывает функцию signal_handler 
    @param signal 
*/
void signal_handler(int signal) {
    if (signal == SIGINT) {
        printf("Server stopped\n");
        running = 0;
        exit(0);
    }
}

int main() {
    signal(SIGINT, signal_handler);

    int chat_fd;
    chat_fd = shm_open(SHM_CHAT, O_CREAT | O_RDWR, 0666);
    if (chat_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    ftruncate(chat_fd, sizeof(chat_t));
    chat_ptr = mmap(0, sizeof(chat_t), PROT_READ | PROT_WRITE, MAP_SHARED, chat_fd, 0);
    if (chat_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    chat_ptr->count = 0;
    sem_chat = sem_open(SEM_CHAT, O_CREAT, 0666, 0);
    if (sem_chat == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    int members_fd;
    members_fd = shm_open(SHM_MEMBERS, O_CREAT | O_RDWR, 0666);
    if (members_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    ftruncate(members_fd, sizeof(members_t));
    members_ptr = mmap(0, sizeof(members_t), PROT_READ | PROT_WRITE, MAP_SHARED, members_fd, 0);
    if (members_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    members_ptr->count = 0;
    sem_members = sem_open(SEM_MEMBERS, O_CREAT, 0666, 0);
    if (sem_members == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    
    int msg_fd;
    msg_fd = shm_open(SHM_MSG, O_CREAT | O_RDWR, 0666);// Создаем сегмент разделяемой памяти
    if (msg_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    ftruncate(msg_fd, sizeof(message_t));// Устанавливаем размер сегмента
    msg_ptr = mmap(0, sizeof(message_t), PROT_READ | PROT_WRITE, MAP_SHARED, msg_fd, 0);// Мапируем сегмент в адресное пространство
    if (msg_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    sem_msg = sem_open(SEM_TEXT, O_CREAT, 0666, 0);
    if (sem_msg == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }  
    
    printf("Server launched.\n");
    while (running){
        sem_wait(sem_msg);// Ожидаем ответа от клиента
        size_t text_len = sizeof(chat_ptr->text[0]);
        char new_line[text_len];
        if (msg_ptr->type == NAME) {
            snprintf(new_line, text_len, "New client:%s\n", msg_ptr->client_name);
            if (members_ptr->count < MAX_CLIENTS) {
                strncpy(members_ptr->names[members_ptr->count], msg_ptr->client_name, MAX_NAME_LEN);
                members_ptr->count++;
            }
            sem_post(sem_members);
            sem_post(sem_chat);
        }
        else if (msg_ptr->type == TEXT) {
            if (strcmp(msg_ptr->text, "exit") == 0) {
                snprintf(new_line, text_len, "Client %s disconnected\n", msg_ptr->client_name);
            }
            else{
                snprintf(new_line, text_len, "%s: %s\n", msg_ptr->client_name, msg_ptr->text);
                // printf("Client's answer:\nTYPE: %ld, TEXT: %s, NAME %s\n", msg_ptr->type, msg_ptr->text, msg_ptr->client_name);
            }
        }
        if (chat_ptr->count < MAX_LINES) {
            snprintf(chat_ptr->text[chat_ptr->count], text_len, "%s", new_line);
            chat_ptr->count++;
        }
        else{
            for (int i = 0; i < MAX_LINES - 1; i++) {
                strncpy(chat_ptr->text[i], chat_ptr->text[i + 1], text_len);
            }
            snprintf(chat_ptr->text[chat_ptr->count], text_len, "%s", new_line);
        }
        sem_post(sem_chat);
    }
    // Удаляем сегмент разделяемой памяти
    munmap(members_ptr, sizeof(members_t));
    munmap(msg_ptr, sizeof(message_t));
    munmap(chat_ptr, sizeof(chat_t));
    // Отключаемся от сегмента
    shm_unlink(SHM_MSG);
    shm_unlink(SHM_MEMBERS);
    shm_unlink(SHM_CHAT);
    // Закрываем семафоры
    sem_close(sem_msg);
    sem_close(sem_members);
    sem_close(sem_chat);
    // Удаляем семафоры
    sem_unlink(SEM_TEXT);
    sem_unlink(SEM_MEMBERS);
    sem_unlink(SEM_CHAT);

    return 0;
}
