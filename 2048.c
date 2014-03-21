#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <ctype.h>

#define debug printf("LINE: %d\n", __LINE__); fflush(stdout);

int PORT = 8080;

enum { SEND_BUF_SIZE = 4096 };

typedef struct threadParams {
    int sockid;
    pthread_mutex_t *mutex;
} threadParams;

volatile int sockid;

char *
socket_gets(int sockid)
{
    char c;
    char *s = calloc(1024, sizeof(*s));
    int r;
    int l = 0;
    while ((r = recv(sockid, &c, 1, 0)) > 0) {
        s[l++] = c;
        if (c == '\n') {
            break;
        }
    }
    s[l] = 0;
    return s;
}

void
hnd(int sig)
{
    shutdown(sockid, SHUT_RDWR);
    close(sockid);
    exit(0);
}

void
hnd2(int sig)
{
    printf("SIGPIPE\n");
}

int allowed_file(char *fn)
{
    if (!strcmp(fn, "./2048.html")) return 1;
    if (!strcmp(fn, "./2048_files/application.js")) return 2;
    if (!strcmp(fn, "./2048_files/game_manager.js")) return 2;
    if (!strcmp(fn, "./2048_files/grid.js")) return 2;
    if (!strcmp(fn, "./2048_files/html_actuator.js")) return 2;
    if (!strcmp(fn, "./2048_files/keyboard_input_manager.js")) return 2;
    if (!strcmp(fn, "./2048_files/local_score_manager.js")) return 2;
    if (!strcmp(fn, "./2048_files/tile.js")) return 2;
    if (!strcmp(fn, "./2048_files/main.css")) return 3;
    if (!strcmp(fn, "./2048_files/fedotov.png")) return 4;
    if (!strcmp(fn, "./2048_files/inovenko.png")) return 4;
    if (!strcmp(fn, "./2048_files/mash.png")) return 4;
    if (!strcmp(fn, "./2048_files/popov.png")) return 4;
    if (!strcmp(fn, "./2048_files/matveev.png")) return 4;
    if (!strcmp(fn, "./2048_files/panf.png")) return 4;
    if (!strcmp(fn, "./2048_files/razborov.png")) return 4;
    if (!strcmp(fn, "./2048_files/roublev.png")) return 4;
    if (!strcmp(fn, "./2048_files/s4g.png")) return 4;
    if (!strcmp(fn, "./2048_files/shagi.png")) return 4;
    if (!strcmp(fn, "./2048_files/shestimerny.png")) return 4;
    if (!strcmp(fn, "./2048_files/pixel.gif")) return 5;
    if (!strcmp(fn, "./favicon.ico")) return 5;
    return 0;
}

char *createHeader(int type, int len) {
    char *buf = calloc(SEND_BUF_SIZE, sizeof(*buf));
    strcat(buf, "HTTP/1.1 200 OK\n");
    strcat(buf, "Server: Vovchik Server\n");
    switch (type) {
    case 1:
        strcat(buf, "Content-type: text/html; charset=utf-8\n");
        break;
    case 2:
        strcat(buf, "Content-type: application/javascript; charset=utf-8\n");
        break;
    case 3:
        strcat(buf, "Content-type: text/css; charset=utf-8\n");
        break;
    case 4:
        strcat(buf, "Content-type: image/png\n");
        strcat(buf, "Cache-control: public, max-age = 3600\n");
        break;
    case 5:
        strcat(buf, "Content-type: image/gif\n");
        strcat(buf, "Cache-control: public, max-age = 3600\n");
        break;
    default:
        break;
    }
    char cl[64];
    memset(cl, 0, 64);
    sprintf(cl, "Content-Length: %d\n", len);
    strcat(buf, cl);
    strcat(buf, "Connection: close\n\n");
    return buf;
}
/*
void send_file(int sockid, char *fn)
{
    unsigned char *addr = calloc(SEND_BUF_SIZE, sizeof(*addr));
    int fd = open(fn, O_RDONLY);
    int ftype;
    if (strstr(fn, "..") || !(ftype = allowed_file(fn))) {
        if (fd != -1) {
            close(fd);
            fd = -1;
        }
    }
    printf("%s\n", fn);
//    if (fd == -1) {
//        fd = open("./404.html", O_RDONLY);
//    }
    if (fd != -1) {
        char *hdr = createHeader(ftype);
        send(sockid, hdr, strlen(hdr), 0);
        free(hdr);
        int r;
        while ((r = read(fd, addr, SEND_BUF_SIZE)) > 0) {
            send(sockid, addr, r, 0);
        }
        close(fd);
    }
    free(addr);
}

char *razborov(char *s)
{
    char *buf = calloc(256, sizeof(*buf));
    buf[0] = 0;
    char *last = NULL, *prev = NULL, *cur;
    int shift = 0;
    while (cur = strstr(s + shift, "%22")) {
        prev = last;
        last = cur;
        shift = (cur - s) + 1;
    }
    if (!last || !prev) {
        return buf;
    }
    prev = prev + 3;
    int len = 0;
    for (cur = prev; cur < last; ++cur) {
        if (isalnum(*cur) || *cur == '_') {
            buf[len] = *cur;
            ++len;
            buf[len] = 0;
        }
        if (len == 255) {
            break;
        }
    }
    return buf;
}*/

void
process(int sockid, pthread_mutex_t *mutex)
{
    char *buf = socket_gets(sockid);
    printf("%s\n", buf);
    shutdown(sockid, SHUT_RD);
    if (strstr(buf, "GET") == buf) {
        char *addr = calloc(SEND_BUF_SIZE, sizeof(*addr));
        sscanf(buf + 3, " %s", addr);
        if (!strcmp(addr, "/")) {
            strcat(addr, "2048.html");
        }
        char *fn = calloc(strlen(addr) + 10, sizeof(*fn));
        fn[0] = '.';
        strcat(fn, addr);
        pthread_mutex_lock(mutex);
        int fd = open(fn, O_RDONLY);
        int ftype = 0;
        if (strstr(fn, "..") || !(ftype = allowed_file(fn))) {
            if (fd != -1) {
                close(fd);
                fd = -1;
            }
        }
        printf("%s\n", fn);
//        if (fd == -1) {
//            free(fn);
//            fn = strdup("./404.html");
//            fd = open(fn, O_RDONLY);
//        }
        if (fd != -1) {
            int fsize = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);
            char *hdr = createHeader(ftype, fsize);
            send(sockid, hdr, strlen(hdr), 0);
            ///printf("%s\n", hdr);
            free(hdr);
            int r;
            while ((r = read(fd, addr, SEND_BUF_SIZE)) > 0) {
                send(sockid, addr, r, 0);
            }
            close(fd);
        }
        free(fn);
        pthread_mutex_unlock(mutex);
        free(addr);
    }
    free(buf);
    shutdown(sockid, SHUT_WR);
    printf("Client disconnected\n");
}

void *
run(void *arg)
{
    threadParams *p = (threadParams *) arg;
    int sockid = p->sockid;
    pthread_mutex_t *mutex = p->mutex;
    free(p);
    process(sockid, mutex);
    close(sockid);
    pthread_exit(NULL);
}

int
main(int argc, char **argv)
{
    if (argc > 1) {
        PORT = atoi(argv[1]);
    }
    signal(SIGINT, hnd);
    signal(SIGPIPE, hnd2);
    sockid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in myaddr;
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(PORT);
    inet_aton("127.0.0.1", &myaddr.sin_addr.s_addr);
    if (bind(sockid, (struct sockaddr *) &myaddr, sizeof(myaddr))) {
        exit(1);
    }
    listen(sockid, 16);
    int sid;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    while (1) {
        sid = accept(sockid, NULL, NULL);
        printf("%d\n", sid);
        if (sid == -1) continue;
        printf("Client connected\n");
        threadParams *params = calloc(1, sizeof(*params));
        params->sockid = sid;
        params->mutex = &mutex;
        pthread_t thread;
        while (pthread_create(&thread, NULL, run, (void *) params)) {
            printf("Process not started!\n");
        }
        pthread_detach(thread);
    }
    shutdown(sockid, SHUT_RDWR);
    close(sockid);
    return 0;
}
