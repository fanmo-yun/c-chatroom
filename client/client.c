#include <ncurses.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include "client_api.c"

void* recv_message(void* arge) {
    ARGES* p = (ARGES*)arge;
    recv_message_box(p->win, p->fd);
    return NULL;
}

void run_client() {
    char name[6];
    ARGES arge;
    initscr();
    if (has_colors() == FALSE) {
        fprintf(stdout, "your terminal isn\'t support color");
        exit(0);
    }
    noecho();
    cbreak();
    start_color();

    WINDOW* loginbox = newwin(6, 35, (getmaxy(stdscr) - 6) / 2, (getmaxx(stdscr) - 35) / 2);
    WINDOW* menubox = newwin(getmaxy(stdscr) - (getmaxy(stdscr) - 2), 0, 0, 0);
    arge.win = newwin(getmaxy(stdscr) - 10, 0, getmaxy(stdscr) - (getmaxy(stdscr) - 2), 0);
    WINDOW* inputbox = newwin(0, 0, getmaxy(stdscr) - 8, 0);

    input_username_box(loginbox, name, sizeof(name));

    pthread_t th;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    int client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    arge.fd = &client_sockfd;
    if(client_sockfd == -1) {
        perror("Error1");
        exit(1);
    }
    
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(ADDRESS);
    
    if (connect(client_sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        perror("Error2");
        exit(2);
    };

    
    if (pthread_create(&th, &attr, recv_message, (void*)&arge)) {
        perror("Error3");
        exit(3);
    };

    show_chatbox(arge.win);
    sendmsg_box(menubox, inputbox, client_sockfd, name);

    endwin();
    close(client_sockfd);
}