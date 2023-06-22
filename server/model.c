#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9091
#define BUFFER_SIZE 1024
#define USERLIMITMAX 20
#define ADDRESS "127.0.0.1"

int user_limit = 0;
int* user_list[USERLIMITMAX] = {NULL};

void delete_user(int* index) {
    for (int i = 0; i < user_limit; i++) {
        if (memcmp(user_list[i], index, sizeof(int)) == 0) {
            int j = i;
            for (; j < user_limit - 1; j++) {
                user_list[j] = user_list[j + 1];
            }
            user_list[j] = NULL;
            break;
        }
        
    }
}

void free_arges(struct sockaddr_in** client_addr, socklen_t** length, pthread_t** th, pthread_attr_t** attr) {
    free(*client_addr);
    *client_addr = NULL;
    free(*length);
    *length = NULL;
    free(*th);
    *th = NULL;
    free(*attr);
    *attr = NULL;
}