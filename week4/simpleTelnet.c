#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>

/**
 * @file simpleTCPServer.c
 * @brief Đề bài
 * Tạo 1 TCP Server để lắng nghe ở port 9999
 * Khi 1 user kết nối tới cổng 9999 trên máy mình -> Gửi message cho server
 * Server dùng system chạy lệnh đó -> Output kết quả vào file `output.txt`
 *
 * @brief Cách chạy chương trình
 * `./simpleTCPServer`: Khởi động server (listen ở port 9999)
 * Mở 1 terminal mới, gõ lệnh: `nc -vv 127.0.0.1 9999` để Kết nối tới host: 127.0.0.1, port: 9999
 * Gõ các lệnh bash ("ls", "ls ../", ...) -> Check file output.txt (Nó sẽ trả kết quả vào file output.txt)
 * Lệnh: `lsof -PiTCP -sTCP:LISTEN`: Liệt kê các cổng đang chạy trên máy
 *
 * @brief BTVN
 * Làm 1 client nối tới server này
 * Auto chạy lệnh 'pwd'
 */

#define INVALID_SOCKET -1
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

int SendPacket(int fd, char *data, int len)
{
    int sent = 0;
    do
    {
        sent += send(fd, data + sent, len - sent, 0);
    } while (sent >= 0 && sent < len);
    return sent;
}

int RecvPacket(int fd, char *data, int maxlen)
{
    int received = 0;
    int block_size = 2;
    int tmp = 0;
    do
    {
        tmp = recv(fd, data + received, block_size, 0);
        received += tmp;
    } while (received >= 0 && received < maxlen && tmp == block_size);
    return received;
}

int main(int argc, char **argv)
{
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sfd != INVALID_SOCKET)
    {
        SOCKADDR_IN saddr, caddr;
        unsigned int clen = sizeof(caddr);
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(9999);
        saddr.sin_addr.s_addr = 0;
        bind(sfd, (SOCKADDR *)&saddr, sizeof(saddr));
        listen(sfd, 10);
        int cfd = accept(sfd, (SOCKADDR *)&caddr, &clen);
        if (cfd != INVALID_SOCKET)
        {
            char *welcome = "Hello my first TCP server\n";
            SendPacket(cfd, welcome, strlen(welcome));

            while (0 == 0)
            {
                char buffer[512] = {0};
                RecvPacket(cfd, buffer, sizeof(buffer));
                if (strncmp(buffer, "exit", 4) == 0)
                {
                    break;
                }
                while (buffer[strlen(buffer) - 1] == '\n')
                {
                    buffer[strlen(buffer) - 1] = 0;
                }
                char command[1024] = {0};
                sprintf(command, "%s > out.txt", buffer);
                system(command);
                FILE *f = fopen("out.txt", "rb");
                fseek(f, 0, SEEK_END);
                int size = ftell(f);
                fseek(f, 0, SEEK_SET);
                char *fdata = (char *)calloc(size, 1);
                fread(fdata, 1, size, f);
                fclose(f);
                SendPacket(cfd, fdata, size);
                free(fdata);
            }
            close(cfd);
        }
        close(sfd);
    }
}
