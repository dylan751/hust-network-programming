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
 * Tạo 1 TCP Server để lắng nghe ở port 5555
 * Khi 1 user kết nối tới cổng 5555 trên máy mình -> Gửi message cho server
 * Server in các message đó ra mãi cho tới khi user nhập "exit"
 *
 * Lệnh: `nc -vv 127.0.0.1 5555`: Kết nối tới host: 127.0.0.1, port: 5555
 * Lệnh: `netstat -anvp tcp | awk 'NR<3 || /LISTEN/'`: Liệt kê các cổng đang chạy trên máy
 */

#define INVALID_SOCKET -1
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

// Modify hàm send(): Gửi lại cho đến khi gửi đc hết số bytes cần gửi
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
    int block_size = 1;
    int tmp = 0;
    do
    {
        tmp = recv(fd, data + received, block_size, 0);
        received += tmp;
    } while (received >= 0 && received < maxlen && received == block_size);

    return received;
}

int main(int argc, char **argv)
{
    // Khởi tạo socket
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sfd != INVALID_SOCKET)
    {
        SOCKADDR_IN saddr, caddr; // saddr: Socket address, caddr: Client address
        unsigned int clen = sizeof(caddr);
        // Cấu hình địa chỉ socket
        saddr.sin_family = AF_INET;   // = AF_INET -> Dùng IPv4
        saddr.sin_port = htons(9999); // Listen ở port 5555
        saddr.sin_addr.s_addr = 0;    // Địa chỉ = 0 -> Bảo máy mở mọi cổng (cứ gửi đến là lắng nghe)
        // saddr.sin_addr.s_addr = net_addr("127.0.0.1"); Chỉ mở 1 cổng tại 127.0.0.1

        // Kết nối tới socket
        bind(sfd, (SOCKADDR *)&saddr, sizeof(saddr));     // Thông báo kết nối, đợi ở cổng nào ntn
        listen(sfd, 10);                                  // 10: Độ dài hàng đợi
        int cfd = accept(sfd, (SOCKADDR *)&caddr, &clen); // cfd: c file descriptor
        if (cfd != INVALID_SOCKET)
        {
            char *welcome = "Hello my first TCP server\n";
            SendPacket(cfd, welcome, strlen(welcome));
            while (0 == 0)
            {
                char buffer[1024] = {0};
                RecvPacket(cfd, buffer, sizeof(buffer));

                // Execute command -> Save to file
                while(buffer[strlen(buffer) - 1] == '\n') {
                    buffer[strlen(buffer) - 1] = 0;
                }
                char command[1024] = {0};
                sprintf(command, "%s > out.txt", buffer);
                system(command);
                FILE* f = fopen("out.txt", "rb");
                fseek(f, 0, SEEK_END);
                int size = ftell(f);
                fseek(f, 0, SEEK_SET);
                char* fdata = (char*)calloc(size, 1);
                fread(fdata, 1, size, f);
                fclose(f);
                // Send all the data
                SendPacket(cfd, fdata, size);
                free(fdata);

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
