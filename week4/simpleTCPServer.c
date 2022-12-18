#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

/**
 * @file simpleTCPServer.c
 * @brief Đề bài
 * Tạo 1 TCP Server để lắng nghe ở port 9999
 * Khi 1 user kết nối tới cổng 9999 trên máy mình -> Gửi message cho server
 * Server in các message đó ra mãi cho tới khi user nhập "exit"
 *
 * @brief Cách chạy chương trình
 * `./simpleTCPServer`: Khởi động server (listen ở port 9999)
 * Mở 1 terminal mới, gõ lệnh: `nc -vv 127.0.0.1 9999` để Kết nối tới host: 127.0.0.1, port: 9999
 * Lệnh: `lsof -PiTCP -sTCP:LISTEN`: Liệt kê các cổng đang chạy trên máy
 * Sang terminal mới, gõ chữ gì thì sẽ hiển thị trên terminal của server (nhập mãi cho đến khi ấn nhập 'exit')
 */

#define INVALID_SOCKET -1
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

// netstat -tupl: Liệt kê các cổng đang chạy trên máy

int main(int argc, char **argv)
{
    // Khởi tạo socket
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sfd != INVALID_SOCKET)
    {
        SOCKADDR_IN saddr, caddr; // saddr: Server address, caddr: Client address
        unsigned int clen = sizeof(caddr);
        // Cấu hình địa chỉ socket
        saddr.sin_family = AF_INET;   // = AF_INET -> Dùng IPv4
        saddr.sin_port = htons(9999); // Listen ở port 9999
        saddr.sin_addr.s_addr = 0;    // Địa chỉ = 0 -> Bảo máy mở mọi cổng (cứ gửi đến là lắng nghe)

        // Kết nối tới socket
        bind(sfd, (SOCKADDR *)&saddr, sizeof(saddr));     // Thông báo kết nối, đợi ở cổng nào ntn
        listen(sfd, 10);                                  // 10: Độ dài hàng đợi
        int cfd = accept(sfd, (SOCKADDR *)&caddr, &clen); // cfd: client file descriptor
        if (cfd != INVALID_SOCKET)
        {
            char *welcome = "Hello my first TCP server\n";
            send(cfd, welcome, strlen(welcome), 0); // Gửi message welcome tới client
            while (0 == 0)
            {
                char buffer[1024] = {0};
                recv(cfd, buffer, sizeof(buffer), 0);
                printf("%s\n", buffer);
                if (strncmp(buffer, "exit", 4) == 0)
                {
                    break;
                }
            }
            close(cfd);
        }
        close(sfd);
    }
}