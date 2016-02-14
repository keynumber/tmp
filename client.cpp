#include <sys/epoll.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_EVENTS 1000
#define BUFFER_SIZE 1000
#define EPOLLEN 1018

uint32_t suc = 0;
uint32_t fail = 0;

int write_to_server(int fd) {
    const char * str = "hello world";
    char buf[BUFFER_SIZE];
    *(uint32_t*)buf = strlen(str);
    memcpy(buf + sizeof(uint32_t), str, strlen(str));

    int ret = write(fd, buf, strlen(str) + sizeof(uint32_t));
    if (ret < 0) {
        puts("write failed");
    }
    return ret;
}

int visit_client()
{
    struct sockaddr_in conn_addr;
    int len = sizeof(struct sockaddr_in);

    int conn = socket(AF_INET, SOCK_STREAM, 0);
    if (conn == -1) {
        perror("socket error\n");
        return -1;
    }

    conn_addr.sin_family = AF_INET;
    conn_addr.sin_port = htons(12345);
    conn_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(conn, (struct sockaddr*)&conn_addr, len) < 0) {
        perror("connect error: ");
        close(conn);
        return -1;
    }
    int ret = write_to_server(conn);
    close(conn);
    return ret;
}

int main(int argc, char * argv[])
{
    int n = atoi(argv[1]);
    printf("will visit server %d times\n", n);
    for (int i=0; i<n; ++i) {
        if (visit_client() < 0) {
            fail++;
        } else {
            suc ++;
        }
    }
    printf("suc: %d\nfail: %d\n", suc, fail);
    return 0;
}
