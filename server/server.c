#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include "model.c"

pthread_mutex_t user_limit_num_lock;

void* user_client(void* connect) {
    char msgbuf[BUFFER_SIZE];
    while (true) {
        memset(msgbuf, 0, sizeof(msgbuf));
        int size = recv(*(int*)connect, msgbuf, sizeof(msgbuf), 0);
        if (size <= 0) {
            break;
        }
        pthread_mutex_lock(&user_limit_num_lock);
        for (int i = 0; i < user_limit; i++) {
            if (send(*user_list[i], msgbuf, size, 0) == -1) {
                perror("send2");
                exit(8);
            };
        }
        pthread_mutex_unlock(&user_limit_num_lock);
    }
    pthread_mutex_lock(&user_limit_num_lock);
    delete_user((int*)connect);
    user_limit -= 1;
    pthread_mutex_unlock(&user_limit_num_lock);
    free(connect);
    connect = NULL;
    return NULL;
}

void run_server() {
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = inet_addr(ADDRESS);
    server_sockaddr.sin_port = htons(PORT);

    if (bind(server_sockfd, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) == -1) {
        perror("bind");
        exit(2);
    }

    if (listen(server_sockfd, 10) == -1) {
        perror("listen");
        exit(3);
    }

    if (pthread_mutex_init(&user_limit_num_lock, NULL)) {
        perror("pthread_mutex_init");
        exit(0);
    }

    while (true) {
        struct sockaddr_in* client_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
        socklen_t* length = (socklen_t*)malloc(sizeof(socklen_t));
        *length = sizeof(*client_addr);
        int* conn = (int*)malloc(sizeof(int));
        *conn = accept(server_sockfd, (struct sockaddr*)client_addr, length);

        if (*conn == -1) {
            perror("accept");
            exit(4);
        }
        if (user_limit < USERLIMITMAX) {
            pthread_mutex_lock(&user_limit_num_lock);
            user_list[user_limit] = conn;
            user_limit += 1;
            pthread_mutex_unlock(&user_limit_num_lock);

            pthread_t* th = (pthread_t*)malloc(sizeof(pthread_t));
            pthread_attr_t* attr = (pthread_attr_t*)malloc(sizeof(pthread_attr_t));
            if (pthread_attr_init(attr)) {
                perror("pthread_attr_init");
                exit(5);
            };
            
            if (pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED)) {
                perror("pthread_attr_setdetachstate");
                exit(6);
            };

            if(pthread_create(th, attr, user_client, (void*)conn)) {
                perror("pthread_create");
                exit(7);
            };

            free_arges(&client_addr, &length, &th, &attr);
        } else {
            if (send(*conn, "FULL", sizeof("FULL"), 0) == -1) {
                perror("send1");
            };
            close(*conn);
            free(conn);
            conn = NULL;
            free(client_addr);
            client_addr = NULL;
            free(length);
            length = NULL;
        }
    }
    if (pthread_mutex_destroy(&user_limit_num_lock)) {
        perror("pthread_mutex_destroy");
        exit(0);
    };
    close(server_sockfd);
}