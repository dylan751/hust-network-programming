#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

/**
 * @file MultiThreadChatRoomServer.c
 * @brief Đề bài
 * Dùng thread để viết 1 TCP Server làm nhiệm vụ:
 * Đợi và nhận kết nối từ client ở cổng 6000
 * Nhận dữ liệu từ 1 client và forward dữ liệu này đến tất cả các client khác đã kết nối
 *
 * @brief Cách chạy chương trình
 * `./MultiThreadChatRoomServer`: Khởi động server (listen ở port 6000)
 * Mở 1 terminal mới, gõ lệnh: `nc -vv 127.0.0.1 6000` để Kết nối tới host: 127.0.0.1, port: 6000
 * Gõ chữ gì thì sẽ echo về client, đồng thời hiển thị "Received: ..." trên terminal của server
 */

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#define MAX_CLIENT 1024
int g_clientfd[MAX_CLIENT];
int g_clientcount = 0;

void *my_func(void *arg)
{
    int cfd = *(int *)arg;
    free(arg);
    while (1)
    {
        char buffer[1024] = {0};
        int r = recv(cfd, buffer, sizeof(buffer), 0);
        if (r > 0)
        {
            printf("Received: %s\n", buffer);
            for (int i = 0; i < g_clientcount; i++)
            {
                send(g_clientfd[i], buffer, strlen(buffer), 0);
            }
        }
    }
}

int main()
{
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    unsigned int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(6000);
    saddr.sin_addr.s_addr = 0;
    bind(sfd, (SOCKADDR *)&saddr, sizeof(saddr));
    listen(sfd, 10);
    while (1)
    {
        int cfd = accept(sfd, (SOCKADDR *)&caddr, &clen);
        g_clientfd[g_clientcount++] = cfd;
        pthread_t pid;
        int *arg = (int *)calloc(1, sizeof(int));
        *arg = cfd;
        pthread_create(&pid, NULL, my_func, arg);
    }
    close(sfd);
}