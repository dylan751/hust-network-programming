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

/**
 * @file BTVN.c
 * @brief Đề bài
 * Fork để tạo 1 tiến trình con chuyên recvfrom từ các client broadcast
 * <TÊN> và lưu thông tin <IP> <TÊN> vào 1 file dùng chung.
 *
 * Khi nhận được <TÊN> của client từ lệnh recvfrom UDP 4000 thì đồng
 * thời gửi lại (echo) IP của server cho client đó trên cổng 7000 (xem bài tập về UDP ECHO đã làm). Client sẽ dùng IP nhận được sau này khi cần
 * lấy danh sách các client khác đang online mà server nắm giữ.
 *
 * Fork để tạo 1 tiến trình con chuyên đợi TCP 5000 và accept: Mỗi lần
 * accept tạo một kết nối và trả về danh sách các <IP> và <TÊN> cho
 * client rồi đóng luôn kết nối.
 *
 * @brief Cách chạy chương trình
 * `./TCPEchoForkServer`: Khởi động server (listen ở port 9999)
 * Mở 1 terminal mới, gõ lệnh: `nc -vv 127.0.0.1 9999` để Kết nối tới host: 127.0.0.1, port: 9999
 * Lệnh: `lsof -PiTCP -sTCP:LISTEN`: Liệt kê các cổng đang chạy trên máy
 * Sang terminal mới, gõ chữ gì thì sẽ hiển thị trên terminal của server và nhận lại callback từ server (nhập mãi cho đến khi ấn nhập 'exit')*
 * Notes: Có thể mở nhiều client cùng kết nối tới 1 server (multi process)
 * 
 * nc -vv -u -l 7000
 */

#define INVALID_SOCKET -1
#define MAX_CLIENT 1024
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

void signhandler(int signum)
{
    int stat = 0;
    // Giải phóng các tiến trình con Zombie
    while (waitpid(-1, &stat, WNOHANG) > 0)
        ;
}

void udp_process()
{
    char *response = "127.0.0.1"; // Không quan trọng vì client có thể tách IP từ recvfrom
    char buffer[1024];
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // Tạo 1 socket UDP
    SOCKADDR_IN saddr, caddr;                          // Socket address, Client address
    unsigned int clen = sizeof(caddr);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(4000);
    saddr.sin_addr.s_addr = 0;

    bind(fd, (SOCKADDR *)&saddr, sizeof(saddr));

    while (0 == 0)
    {
        memset(buffer, 0, sizeof(buffer)); // Trước khi recvfrom -> Reset buffer về 0
        recvfrom(fd, buffer, sizeof(buffer), 0, (SOCKADDR *)&caddr, &clen);

        // Cắt dấu xuống dòng ở cuối buffer đi
        while (buffer[strlen(buffer) - 1] == '\r' || buffer[strlen(buffer) - 1] == '\n')
        {
            buffer[strlen(buffer) - 1] = 0;
        }

        caddr.sin_port = htons(7000);
        sendto(fd, response, strlen(response), 0, (SOCKADDR *)&caddr, clen);

        // Lưu thông tin vào file
        FILE *f = fopen("client.txt", "a+t");
        fprintf(f, "%s %s\n", inet_ntoa(caddr.sin_addr), buffer); // Chuyển IP thành dạng đọc được
        fclose(f);
        close(fd);
    }
}

void tcp_process()
{
    char buffer[1024];
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Tạo 1 socket UDP
    SOCKADDR_IN saddr, caddr;                           // Socket address, Client address
    unsigned int clen = sizeof(caddr);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5000);
    saddr.sin_addr.s_addr = 0;

    bind(fd, (SOCKADDR *)&saddr, sizeof(saddr));
    listen(fd, 10);

    while (0 == 0)
    {
        int cfd = accept(fd, (SOCKADDR *)&caddr, &clen);
        FILE *f = fopen("clients.txt", "rt");

        while (!feof(f))
        {
            char buffer[1024] = {0}; // Buffer cho 1 dòng
            fgets(buffer, sizeof(buffer), f);
            send(cfd, buffer, strlen(buffer), 0);
        }
        fclose(f);
    }
}

int main()
{
    signal(SIGCHLD, signhandler);

    // Tiến trình con thứ 1
    if (fork() == 0)
    {
        udp_process();
    }

    // Tiến trình con thứ 2
    if (fork() == 0)
    {
        tcp_process();
    }

    // Tiến trình cha cho sleep -> đảm bảo không kết thúc trừ khi Ctrl+C
    while (0 == 0)
    {
        sleep(1000);
    }
}