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

#define INVALID_SOCKET -1
#define MAX_CLIENT 32
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

char filename[1024] = {0};
char get[10] = {0};
int port = -1;

int g_clientfd[MAX_CLIENT];
int g_clientcount = 0;

void sighandler(int signum)
{
    int stat = 0;
    // Xoá các tiến trình con zombie
    while (waitpid(-1, &stat, WNOHANG) > 0)
        ;
}

// Gửi đến khi nào hết nội dung (đề phòng trường hợp lỗi)
int SendPacket(int fd, char *data, int len)
{
    int sent = 0;
    do
    {
        sent += send(fd, data + sent, len - sent, 0);
    } while (sent >= 0 && sent < len);
    return sent;
}

/* Note: Có thể cổng 5555 đã có service dùng rồi -> đổi dòng `58` từ 5555 -> 6101*/
/**
 * @brief Đề bài
 * Xem file PDF
 * 
 * @brief Cách chạy chương trình
 * `./midTerm`: Khởi động server (listen ở port 5555)
 * Mở 1 terminal mới, gõ lệnh: `nc -vv -u 127.0.0.1 5555` để Kết nối tới host: 127.0.0.1, port: 5555* 
 * Gõ `GET test.txt 6102`
 * Mở 1 terminal mới, gõ lệnh: `nc -vv -u 127.0.0.1 6102` -> Sẽ nhận được nội dung file test.txt

 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char **argv)
{
    signal(SIGCHLD, sighandler);

    // Khởi tạo socket
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sfd != INVALID_SOCKET)
    {
        SOCKADDR_IN saddr, caddr, faddr; // saddr: Server address, caddr: Client address, faddr: File address
        unsigned int clen = sizeof(caddr);
        // Cấu hình địa chỉ socket
        saddr.sin_family = AF_INET;   // = AF_INET -> Dùng IPv4
        saddr.sin_port = htons(5555); // Listen ở port 5555
        saddr.sin_addr.s_addr = 0;    // Địa chỉ = 0 -> Bảo máy mở mọi cổng (cứ gửi đến là lắng nghe)

        // Kết nối tới socket
        bind(sfd, (SOCKADDR *)&saddr, sizeof(saddr));
        listen(sfd, 32);

        while (0 == 0)
        {
            int cfd = accept(sfd, (SOCKADDR *)&caddr, &clen);
            // Lệnh fork(): Trả về 0 nếu là tiến trình con, trả về id khác nếu là tiến trình cha
            if (fork() == 0)
            {
                close(sfd);
                char *welcome = "Welcome to my server!\n";
                SendPacket(cfd, welcome, strlen(welcome)); // Gửi message welcome tới client
                while (0 == 0)
                {
                    char buffer[1024] = {0};
                    int r = recv(cfd, buffer, sizeof(buffer), 0);
                    if (r > 0)
                    {
                        // Nếu client gửi "QUIT" -> đóng socket giao tiếp với client đó
                        if (strncmp(buffer, "QUIT", 4) == 0)
                        {
                            close(cfd);
                        }
                        else if (strncmp(buffer, "GET", 3) == 0)
                        {
                            sscanf(buffer, "%s%s%d", get, filename, &port);
                            printf("File name: %s\n", filename);
                            printf("Port: %d\n", port);

                            // Server lắng nghe ở cổng port
                            printf("Reading file ...\n");
                            if (port != -1)
                            {
                                faddr.sin_family = AF_INET;
                                faddr.sin_port = htons(port);
                                faddr.sin_addr.s_addr = 0;
                                int ffd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                                bind(ffd, (SOCKADDR *)&faddr, sizeof(faddr));
                                listen(ffd, 32);
                                printf("Listening on port %d\n", port);
                                // Đọc file -> gửi về client
                                while (0 == 0)
                                {

                                    int dtfd = accept(ffd, (SOCKADDR *)&caddr, &clen); // dtfd: data file descriptor
                                    if (dtfd != INVALID_SOCKET)
                                    {
                                        FILE *f = fopen(filename, "rt");
                                        while (!feof(f))
                                        {
                                            char buffer[1024] = {0}; // Buffer cho 1 dòng
                                            fgets(buffer, sizeof(buffer), f);
                                            SendPacket(dtfd, buffer, strlen(buffer));
                                        }
                                        fclose(f);

                                        SendPacket(cfd, "DONE", 4);
                                        close(cfd);
                                        close(dtfd);
                                        close(sfd);
                                        close(ffd);
                                    }
                                }
                            }
                        }
                        // Gửi response thông báo câu lệnh không hợp lệ
                        else
                        {
                            char *response = "INVALID COMMAND\n";
                            SendPacket(cfd, response, strlen(response));
                        }
                    }
                }
            }
            else
            {
                // Luu lai 1 danh sach
                g_clientfd[g_clientcount++] = cfd;
                printf("Client count: %d\n", g_clientcount);
                close(cfd);
            }
        }
        close(sfd);
        return 0;
    }
}