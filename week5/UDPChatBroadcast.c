#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * @file UDPChatServer.c
 * @brief Đề bài
 * Tạo 1 UDP Server để lắng nghe ở port 5000
 * Kết nối tới server -> Broadcast nội dung chat tới tất cả các máy
 *
 * @brief Cách chạy chương trình
 * `./UDPChatServer`: Khởi động server (listen ở port 5000)
 * Mở 1 terminal mới: `nc -vv -u <địa chỉ server> (127.0.0.1) 5000` (Gửi message)
 * Mở 1 terminal mới: `nc -vv -u -l 7000` (Để listen on port 7000, nhận nội dung chat mà server gửi broadcast) (Bất kỳ ai kết nối đến cổng 7000 đều sẽ nhận đc gói tin)
 * Lệnh: `lsof -PiTCP -sTCP:LISTEN`: Liệt kê các cổng đang chạy trên máy
 */

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

int main()
{
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    SOCKADDR_IN saddr, baddr, caddr;
    int num = 0;
    unsigned int clen = sizeof(caddr);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(7000);
    saddr.sin_addr.s_addr = inet_addr("255.255.255.255");

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5000);
    saddr.sin_addr.s_addr = 0;
    bind(s, (SOCKADDR *)&saddr, sizeof(saddr));

    // Set Socket Broadcast option
    int on = 1;
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

    while (1)
    {
        char buffer[1024] = {0};
        int r = recvfrom(s, buffer, sizeof(buffer), 0, (SOCKADDR *)&caddr, &clen);
        if (r > 0)
        {
            printf("%s\n", buffer);
            sendto(s, buffer, strlen(buffer), 0, (SOCKADDR *)&baddr, sizeof(baddr));
        }
        else
            break;
    }
    close(s);
}