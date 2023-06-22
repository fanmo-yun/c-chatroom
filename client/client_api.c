#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <ncurses.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define ESC 27
#define PORT 9091
#define BACKSPACE 127
#define MAX_MSG_SIZE 31
#define BUFFER_SIZE 1000
#define ADDRESS "127.0.0.1"
#define MAX_BUFFER_SIZE 1024
#define ctrl(ch) (ch & 0x1F)

typedef enum type { file, msg } TYPE;

pthread_mutex_t lock_win;

int movey_i = 1;

typedef struct client {
    TYPE t;
    int size;
    char buffer[BUFFER_SIZE];
    char username[6];
    char filename[10];
}Message;

typedef struct arges {
    int* fd;
    WINDOW* win;
}ARGES;

bool file_exist(char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return FALSE;
    } else {
        fclose(fp);
        return TRUE;
    }
}

void savefile(Message* p) {
    FILE* fp = fopen(p->filename, "a+");
    fwrite(p->buffer, sizeof(char), p->size, fp);
    fclose(fp);
}

void sendchr(Message* p, char* mg, char* nm, int fd) {
    p->t = msg;
    memcpy(p->buffer, mg, strlen(mg));
    memcpy(p->username, nm, strlen(nm));
    if (send(fd, (char*)p, sizeof(*p), 0) == -1) {
        perror("Error4");
        exit(4);
    };
}

int sendfile(Message* p, char* nm, char* fn, int fd) {
    if (file_exist(fn) == TRUE) {
        FILE* fp = fopen(fn, "r");
        int fsize;
        while ((fsize = fread(p->buffer, sizeof(char), BUFFER_SIZE, fp)) > 0) {
            p->t = file;
            memcpy(p->filename, fn, strlen(fn));
            memcpy(p->username, nm, strlen(nm));
            p->size = fsize;
            if (send(fd, (char*)p, sizeof(*p), 0) == -1) {
                perror("Error4");
                exit(4);
            };
            memset(p, 0, sizeof(*p));
        }
        fclose(fp);
        return 0;
    }
    return -1;
}

void menubox_style1(WINDOW* win, int* opt) {
    *opt = msg;
    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 1, 1, "CHAT");
    wattroff(win, COLOR_PAIR(1));
    mvwprintw(win, 1, 6, "FILE");
}

void menubox_style2(WINDOW* win, int* opt) {
    *opt = file;
    mvwprintw(win, 1, 1, "CHAT");
    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 1, 6, "FILE");
    wattroff(win, COLOR_PAIR(1));
}

int insert_message(WINDOW* win, void* data) {
    Message* p = (Message*)data;
    if (movey_i <= (getmaxy(stdscr) - 10) - 2) {
        mvwprintw(win, movey_i, 1, "%s: %s", p->username, p->buffer);
        movey_i += 1;
        wrefresh(win);
    } else {
        movey_i = 1;
        wmove(win, 0, 0);
        wclrtobot(win);
        box(win, 0, 0);
        mvwprintw(win, movey_i, 1, "%s: %s", p->username, p->buffer);
        wrefresh(win);
        movey_i += 1;
    }
    return 0;
}

void show_chatbox(WINDOW* win) {
    box(win, 0, 0);
    wrefresh(win);
}

void input_username_box(WINDOW* win, char* name, int size) {
    int ch, i = 0;
    const char context[21] = "Enter your username:";
    box(win, 0, 0);
    mvwprintw(win, 1, (35 - strlen(context)) / 2, "%s", context);
    refresh();
    wrefresh(win);
    while ((ch = getch()) != '\n') {
        switch (ch) {
            case BACKSPACE:
                if (i > 0 && i < size) {
                    i -= 1;
                    name[i] = 0;
                    mvwprintw(win, 2, 7, "%s", name);
                    wclrtoeol(win);
                    box(win, 0, 0);
                    wrefresh(win);
                }
                break;

            default:
                if (isprint(ch) && i < size - 1) {
                    name[i] = ch;
                    name[i + 1] = 0;
                    mvwprintw(win, 2, 7, "%s", name);
                    wrefresh(win);
                    i += 1;
                }
                break;
        }
    }
    if (strlen(name) == 0) {
        memcpy(name, "nname", 5);
    }
}

void sendmsg_box(WINDOW* win1, WINDOW* win2, int fd, char* name) {
    int ch, i = 0, option = msg, running = TRUE;
    init_pair(1, COLOR_BLACK, COLOR_WHITE);

    box(win1, 0, 0);
    menubox_style1(win1, &option);
    wrefresh(win1);

    box(win2, 0, 0);
    mvwprintw(win2, 0, 1, "input:");
    wrefresh(win2);

    Message msge;
    char message[MAX_MSG_SIZE];
    memset(&msge, 0, sizeof(msge));
    memset(message, 0, MAX_MSG_SIZE);
    while (running) {
        ch = getch();
        switch (ch) {
            case ESC:
            case '\t':
                running = FALSE;
                break;

            case '\n':
                if (strlen(message) != 0) {
                    if (option == msg) {
                        i = 0;
                        sendchr(&msge, message, name, fd);
                        memset(&msge, 0, sizeof(msge));
                        memset(message, 0, MAX_MSG_SIZE);
                        wmove(win2, 1, 2);
                        wclrtoeol(win2);
                        box(win2, 0, 0);
                        mvwprintw(win2, 0, 1, "input:");
                        wrefresh(win2);
                    } else {
                        i = 0;
                        sendfile(&msge, name, message, fd);
                        memset(&msge, 0, sizeof(msge));
                        memset(message, 0, MAX_MSG_SIZE);
                        wmove(win2, 1, 2);
                        wclrtoeol(win2);
                        box(win2, 0, 0);
                        mvwprintw(win2, 0, 1, "input:");
                        wrefresh(win2);
                    }
                }
                break;

            case ctrl('a'):
                menubox_style1(win1, &option);
                wrefresh(win1);
                break;
            
            case ctrl('f'):
                menubox_style2(win1, &option);
                wrefresh(win1);
                break;

            case BACKSPACE:
                if (i > 0 && i <= MAX_MSG_SIZE) {
                    i -= 1;
                    message[i] = 0;
                    wclrtoeol(win2);
                    box(win2, 0, 0);
                    mvwprintw(win2, 0, 1, "input:");
                    mvwprintw(win2, 1, 2, "%s", message);
                    wrefresh(win2);
                }
                break;
            
            default:
                if (option == msg && isprint(ch) && i < MAX_MSG_SIZE - 1) {
                    message[i] = ch;
                    message[i + 1] = 0;
                    i += 1;
                    mvwprintw(win2, 1, 2, "%s", message);
                    wrefresh(win2);
                }
                if (option == file && isprint(ch) && i < 9) {
                    message[i] = ch;
                    message[i + 1] = 0;
                    i += 1;
                    mvwprintw(win2, 1, 2, "%s", message);
                    wrefresh(win2);
                }
                break;
        }
    }
}

void recv_message_box(WINDOW* win, int *fd) {
    char recvbuf[MAX_BUFFER_SIZE];
    Message recv_msge;
    while (TRUE) {
        memset(recvbuf, 0, sizeof(recvbuf));
        memset(&recv_msge, 0, sizeof(recv_msge));
        int size = recv(*fd, recvbuf, sizeof(recvbuf), 0);
        if (size <= 0 || strcmp(recvbuf, "FULL") == 0) {break;}
        else {
            memcpy(&recv_msge, recvbuf, sizeof(recvbuf));
            if (recv_msge.t == msg) {
                pthread_mutex_lock(&lock_win);
                use_window(win, insert_message, (void*)&recv_msge);
                pthread_mutex_unlock(&lock_win);
            } else {
                savefile(&recv_msge);
            }
        }
    }
}