#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

/**
 * @file simpleUDPServer.c
 * @brief Đề bài
 * Tạo 1 UDP Server để lắng nghe ở port 6101
 * Khi 1 user kết nối tới cổng 6101 trên máy mình -> Gửi message cho server
 *
 * @brief Cách chạy chương trình
 * `./simpleUDPServer`: Khởi động server (listen ở port 9999)
 * Mở 1 terminal mới, gõ lệnh: `nc -vv -u 127.0.0.1 6101` để Kết nối tới host: 127.0.0.1, port: 6101
 * Lệnh: `lsof -PiTCP -sTCP:LISTEN`: Liệt kê các cổng đang chạy trên máy
 * Sang terminal mới, gõ chữ gì thì sẽ hiển thị trên terminal của server (nhập mãi cho đến khi ấn nhập 'exit')
 */

#define INVALID_SOCKET -1
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

// netstat -tupl: Liệt kê các cổng đang chạy trên máy

int main(int argc, char **argv)
{
    // Khởi tạo UDP socket
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    SOCKADDR_IN saddr, caddr;
    unsigned int clen = sizeof(caddr);

    // Cấu hình địa chỉ socket
    saddr.sin_family = AF_INET;   // = AF_INET -> Dùng IPv4
    saddr.sin_port = htons(6101); // Listen ở port 6101: Cổng mà phần mềm của thầy đang chạy (lắng nghe mọi gói tin từ cổng 6101)
    saddr.sin_addr.s_addr = 0;    // Địa chỉ = 0 -> Bảo máy mở mọi cổng (cứ gửi đến là lắng nghe)

    // Kết nối tới socket
    bind(s, (SOCKADDR *)&saddr, sizeof(saddr));
    while (1)
    {
        char buffer[1024] = {0};
        int r = recvfrom(s, buffer, sizeof(buffer), 0, (SOCKADDR *)&caddr, &clen);
        if (r > 0)
        {
            printf("%s\n", buffer);
            // Callback: Server nhận dữ liệu r gửi lại dữ liệu cho phía Client
            caddr.sin_port = htons(5000); // Cố định gửi lại dữ liệu vào cổng 5000 (Check code phía client đang bind() tới cổng này)
            sendto(s, buffer, strlen(buffer), 0, (SOCKADDR *)&caddr, sizeof(caddr));
        }
        else
        {
            break;
        }
    }
    close(s);
}