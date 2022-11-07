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
 * Nhận gói tin đăng ký ở cổng 5000, khi nhận gói tin "REG xxxxxxx" thì lưu lại địa chỉ của client gửi đến vào danh sách
 * Nhận gói tin chat ở cổng 5000, khi nhận được "CHAT xxxxxx" thì gửi lại cho tất cả các client đã đăng ký (cổng 7000)
 * Nhận gói tin không bắt đầu bởi REG và CHAT thì bỏ qua
 *
 * @brief Cách chạy chương trình
 * `./UDPChatServer`: Khởi động server (listen ở port 5000)
 * 
 * Mở 1 terminal mới: `nc -vv -u <địa chỉ server> (127.0.0.1) 5000` (để gửi gói đăng ký xong rồi gửi gói tin chat)
 * Gửi gói đăng kí: `REG Duong`
 * Gửi chat: `CHAT Hello`
 * 
 * Mở 1 terminal mới: `nc -vv -u -l 7000` (để listen ở port 7000, nhận nội dung chat của các client khác mà server gửi)
 * Lệnh: `lsof -PiTCP -sTCP:LISTEN`: Liệt kê các cổng đang chạy trên máy
 */

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

int main()
{
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    SOCKADDR_IN saddr, caddr, ackaddr;
    SOCKADDR_IN client[100];
    int num = 0;
    unsigned int clen = sizeof(caddr);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5000);
    saddr.sin_addr.s_addr = 0;
    bind(s, (SOCKADDR *)&saddr, sizeof(saddr));
    while (1)
    {
        char buffer[1024] = {0};
        int r = recvfrom(s, buffer, sizeof(buffer), 0, (SOCKADDR *)&caddr, &clen);
        if (r > 0)
        {
            printf("%s\n", buffer);
            if (strncmp(buffer, "REG", 3) == 0)
            {
                client[num++] = caddr;
                printf("REG: %d: %d\n", num - 1, caddr.sin_addr.s_addr);
            }
            else if (strncmp(buffer, "CHAT", 4) == 0)
            {
                for (int i = 0; i < num; ++i)
                {
                    client[i].sin_port = htons(7000);
                    sendto(s, buffer + 5, strlen(buffer + 5), 0, (SOCKADDR *)&(client[i]), sizeof(client[i]));
                }
            }
        }
        else
            break;
    }
    close(s);
}