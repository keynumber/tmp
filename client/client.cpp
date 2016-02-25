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

#include <iostream>
#include <random>

#include "server/net_complete_func.h"

using namespace std;

const unsigned short server_port = 12345;
const char * server_addr = "127.0.0.1";

uint32_t suc = 0;
uint32_t fail = 0;

int generator_sendbuf(char *buf, int len)
{
    const char * str[] = {"one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
    int str_len = sizeof(str)/sizeof(char*);

    memset(buf, 0, len);

    ef::PacketHeader * header = (ef::PacketHeader*)buf;
    random_device rd;
    int t = rd() % str_len;
    strcat(header->payload, str[t]);

    int content_len = strlen(header->payload);
    header->length = content_len;
    header->request_id = rd();
    cout << "content_len: " << header->length << ", request_id: " << header->request_id << ", ";
    cout << "content: " << header->payload << endl;
    return sizeof(ef::PacketHeader) + content_len;
}

int write_to_server(int fd, char *buf, int len) {
    int send_len = len;
    int total_send = 0;
    while (true) {
        int ret = write(fd, buf + total_send, send_len);
        if (ret < 0) {
            perror("write to server failed, errmsg: ");
            return -1;
        }
        total_send += send_len;
        if (total_send == send_len) {
            break;
        }
    }
    printf("totoal send len %d\n", len);
    return 0;
}

int read_from_server(int clientfd, char *buf, int len)
{
    memset(buf, 0, len);

    int packet_len = 0;
    int total_read = 0;
    while(true) {
        int ret = read(clientfd, buf + total_read, len);
        if (ret < 0) {
            perror("read from server failed, errmsg: ");
            return -1;
        }
        total_read += ret;
        ret = ef::packet_len_func(buf, total_read, &packet_len);
        if (ret < 0) {
            printf("packet_len_func error\n");
            break;
        } else if (ret > 0){
            return ret;
        }
    }
    return -1;
}

int request_server()
{
    struct sockaddr_in conn_addr;
    int len = sizeof(struct sockaddr_in);

    int conn = socket(AF_INET, SOCK_STREAM, 0);
    if (conn == -1) {
        perror("socket error\n");
        return -1;
    }

    conn_addr.sin_family = AF_INET;
    conn_addr.sin_port = htons(server_port);
    conn_addr.sin_addr.s_addr = inet_addr(server_addr);
    if (connect(conn, (struct sockaddr*)&conn_addr, len) < 0) {
        perror("connect error: ");
        close(conn);
        return -1;
    }

    const int buflen = 10240;
    char buf[buflen];
    len = generator_sendbuf(buf, buflen);
    int ret = write_to_server(conn, buf, len);
    if (read_from_server(conn, buf, buflen) > 0) {
        ef::PacketHeader * header = (ef::PacketHeader*)buf;
        printf("read from server length %d, request_id %d, payload: %s\n",
                header->length, header->request_id, header->payload);
    }
    close(conn);
    return ret;
}

int main(int argc, char * argv[])
{
    int n = atoi(argv[1]);
    for (int i=0; i<n; ++i) {
        if (request_server() < 0) {
            fail++;
        } else {
            suc ++;
        }
    }
    printf("suc: %d\nfail: %d\n", suc, fail);
    return 0;
}
