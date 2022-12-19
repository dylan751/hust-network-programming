#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

/**
 * @file simpleUDPClient.c
 * @brief Đề bài
 * Tạo 1 UDP Client để gửi dữ liệu
 * Nhận lại dữ liệu callback từ Server
 *
 * @brief Cách chạy chương trình
 * `./simpleUDPServer`: Khởi động server (listen ở port 6101)
 * Mở 1 terminal mới, `./simpleUDPClient`: Khởi động client
 * Gõ chữ vào client -> Check server xem có nhận được không
 * Check Client xem có nhận lại `ACK: Call back` từ Server không
 *
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

    SOCKADDR_IN saddr, ackaddr; // ackaddr: Địa chỉ nhận dữ liệu từ Server callback về

    // Cấu hình địa chỉ socket nhận dữ liệu
    ackaddr.sin_family = AF_INET;
    ackaddr.sin_port = htons(5000); // Nhận lại dữ liệu từ Server ở cổng 5000
    ackaddr.sin_addr.s_addr = 0;

    bind(s, (SOCKADDR *)&ackaddr, sizeof(ackaddr));

    // Cấu hình địa chỉ socket gửi dữ liệu
    saddr.sin_family = AF_INET;                     // = AF_INET -> Dùng IPv4
    saddr.sin_port = htons(6101);                   // Listen ở port 6101: Cổng mà phần mềm của thầy đang chạy (lắng nghe mọi gói tin từ cổng 6101)
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Địa chỉ để gửi dữ liệu đến, địa chỉ gửi đến thì phải rõ ràng (kp '0')

    // Liên tục lấy từ bàn phím -> Gửi đi
    while (1)
    {
        char buffer[1024] = {0};
        fgets(buffer, sizeof(buffer), stdin);
        int sent = sendto(s, buffer, strlen(buffer), 0, (SOCKADDR *)&saddr, sizeof(saddr));

        // Nhận phản hồi từ Server -> In ra màn hình
        char ack[1024] = {0};
        recvfrom(s, ack, sizeof(ack), 0, NULL, NULL);
        printf("ACK: %s\n", ack);
    }
    close(s);
}