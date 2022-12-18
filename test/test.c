/**
 * @file simpleUDPServer.c
 * @brief Đề bài
 * Tạo 1 UDP Server để lắng nghe ở port 6101
 * Khi 1 user kết nối tới cổng 6101 trên máy mình -> Gửi message cho server
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>

#define INVALID_SOCKET -1
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

int main()
{
    int sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKADDR_IN saddr, caddr;
    unsigned int clen = sizeof(caddr);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(6101);
                         saddr.sin_addr.s_addr = 0;

    bind(sfd, (SOCKADDR *)&saddr, sizeof(saddr));

    while (1)
    {

        char buffer[1024] = {0};
        int r = recvfrom(sfd, buffer, sizeof(buffer), 0, (SOCKADDR *)&caddr, &clen);
        if (r >= 0)
        {
            printf("%s\n", buffer);
            sendto(sfd, buffer, sizeof(buffer), 0, (SOCKADDR *)&caddr, clen);
        }
        else
        {
            break;
        }
    }
    close(sfd);
}