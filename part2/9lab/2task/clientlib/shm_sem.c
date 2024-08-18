#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

#include "clientlib.h"

bool running = true;

/** @brief Функция для записи сообщений в разделяемую память
    @param arg - не используется
    @return NULL
*/
void *write_to_shm(void *arg) {
    sem_t *sem_text;
    message_t *msg_ptr;
    int msg_fd;
    msg_fd = shm_open(SHM_MSG, O_RDWR, 0666);// Подключаемся к существующему сегменту разделяемой памяти
    if (msg_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    msg_ptr = mmap(0, sizeof(message_t), PROT_WRITE, MAP_SHARED, msg_fd, 0);// Мапируем сегмент в адресное пространство
    if (msg_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    sem_text = sem_open(SEM_TEXT, 0);
    if (sem_text == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    
    char name[MAX_NAME_LEN];
    printf("Input your name: ");
    fgets(msg_ptr->client_name, MAX_NAME_LEN-1, stdin);
    msg_ptr->client_name[strcspn(name, "\n")] = 0;
    msg_ptr->type = NAME;
    sem_post(sem_text);

    msg_ptr->type = TEXT;
    while (1) {
        printf("Message text: ");
        fgets(msg_ptr->text, MAX_SIZE, stdin);
        msg_ptr->text[strcspn(msg_ptr->text, "\n")] = 0;
        sem_post(sem_text);
        if (strcmp(msg_ptr->text, "exit") == 0) {
            running = false;
            break;
        }
    }
    running = false;
    // Удаляем сегмент разделяемой памяти
    munmap(msg_ptr, sizeof(message_t));
    // Отключаемся от сегмента
    shm_unlink(SHM_MSG);
    // Закрываем семафоры
    sem_close(sem_text);
    // Удаляем семафоры
    sem_unlink(SEM_TEXT);
    return NULL;
}

/** @brief Функция для отслеживания списка участников чата
    @param arg - не используется
    @return NULL
*/
void *members_control(void *arg) {
    int members_fd;
    members_fd = shm_open(SHM_MEMBERS, O_RDWR, 0666);// Подключаемся к существующему сегменту разделяемой памяти
    if (members_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    members_t *members_ptr;
    members_ptr = mmap(0, sizeof(members_t), PROT_READ, MAP_SHARED, members_fd, 0);// Мапируем сегмент в адресное пространство
    if (members_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    sem_t *sem_members;
    sem_members = sem_open(SEM_MEMBERS, 0); 
    if (sem_members == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    sem_wait(sem_members);
    printf("Members:\n");
    for (int i = 0; i < members_ptr->count; i++) {
        printf("\t%s\n", members_ptr->names[i]);
    }
    munmap(members_ptr, sizeof(members_t));
    shm_unlink(SHM_MEMBERS);
    sem_close(sem_members);
    sem_unlink(SEM_MEMBERS);
    return NULL;
}

/** @brief Функция для отслеживания сообщений чата
    @param arg - не используется
    @return NULL
*/
void *chat_control(void *arg) {
    int chat_fd;
    chat_fd = shm_open(SHM_CHAT, O_RDWR, 0666);
    if (chat_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    chat_t *chat_ptr;
    chat_ptr = mmap(0, sizeof(chat_t), PROT_READ, MAP_SHARED, chat_fd, 0);
    if (chat_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    sem_t *sem_chat;
    sem_chat = sem_open(SEM_CHAT, 0);
    if (sem_chat == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    while (running) {
        sem_wait(sem_chat);
        for (int i = 0; i < chat_ptr->count; i++)
            printf("%s\n", chat_ptr->text[i]);
    }
    munmap(chat_ptr, sizeof(chat_t));
    shm_unlink(SHM_CHAT);
    sem_close(sem_chat);
    sem_unlink(SEM_CHAT);
    return NULL;
}
