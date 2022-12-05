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
 * Đợi và nhận kết nối từ client ở cổng 5000
 * Nhận dữ liệu từ 1 client và forward dữ liệu này đến tất cả các client khác đã kết nối
 * @return int
 */

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#define MAX_CLIENT 1024

int g_cfd[MAX_CLIENT] = {0}; // Client file descriptior
int g_count = 0;             // So luong client
int sfd = 0;                 // Socket lễ tân

int main()
{
    sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr;
    saddr.sin_port = htons(8888);
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = 0;
    bind(sfd, (SOCKADDR *)&saddr, sizeof(saddr));
}