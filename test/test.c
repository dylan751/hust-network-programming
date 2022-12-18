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
 * @brief Đề bài
 * Dùng lệnh system chạy lệnh "ls -a -l"
 * -> sau đó điều hướng ra file (ví dụ: ls.out).
 * Đọc file ls.out và phân tích để đưa ra danh sách các file (CHỈ CÓ FILE, KHÔNG CÓ THƯ MỤC) theo định dạng:
 * <Tên File 1> <Kích thước>
 * <Tên File 2> <Kích thước>
 * <Tên File 3> <Kích thước>
 * <Tên File 4> <Kích thước>
 * ..
 */

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;

int main()
{
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    SOCKADDR_IN saddr, caddr, ackaddr;
    SOCKADDR_IN client[1024];
    unsigned int clen = sizeof(caddr);
    int client_count = 0;

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
                client[client_count] = caddr;
                client_count++;
                printf("REG number: %d - s_addr: %d\n", client_count - 1, caddr.sin_addr.s_addr);
            }
            if (strncmp(buffer, "CHAT", 4) == 0)
            {
                for (int i = 0; i < client_count; i++)
                {
                    client[i].sin_port = htons(7000);
                    sendto(s, buffer, sizeof(buffer), 0, (SOCKADDR *)&client[i], sizeof(client[i]));
                }
            }
        }
    }
}