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
#include <pthread.h>

/**
 * @file MultiThreadChatRoomServer.c
 * @brief Đề bài
 * Dùng thread để viết 1 TCP Server làm nhiệm vụ:
 * Đợi và nhận kết nối từ client ở cổng 5000
 * Nhận dữ liệu từ 1 client và forward dữ liệu này đến tất cả các client khác đã kết nối
 * @return int
 */

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#define MAX_CLIENT 1024

int g_cfd[MAX_CLIENT] = {0}; // Client file descriptior
int g_count = 0;             // So luong client
int sfd = 0;                 // Socket lễ tân

// Hàm nối 1 xâu vào 1 xâu khác
void Append(char **pdst, const char *src)
{
    // Xâu ký tự cũ
    char *dst = *pdst;
    // Tính độ dài ban đầu của xâu
    int oldLen = (dst == NULL ? 0 : strlen(dst));
    // Độ dài mới
    int newLen = oldLen + strlen(src) + 1; // Note: Đối với xâu nên cấp phát thêm 1 byte (làm byte lính canh ở cuối)
    dst = (char *)realloc(dst, newLen);
    // Set tất cả bytes từ cuối xâu cũ = 0 (Set các bytes mới cấp phát = 0)
    memset(dst + oldLen, 0, strlen(src) + 1);
    // Nối xâu vào cuối xâu ban đầu
    sprintf(dst + oldLen, "%s", src);

    // Dưa lại giá trị dst vào pdst
    *pdst = dst;
}

// So sánh 2 cấu trúc thư mục (là kiểu directory entries)
int Compare(const struct dirent **a, const struct dirent **b)
{
    if ((*a)->d_type == (*b)->d_type)
        return 0;
    else if ((*a)->d_type == DT_DIR)
        return -1;
    else
        return 1;
}

void *ClientThread(void *arg)
{
    char *rootPath = (char *)calloc(2, sizeof(char));
    strcpy(rootPath, "");

    int cfd = *((int *)arg);
    free(arg);
    arg = NULL;
    // Lien tuc nhan du lieu tu` Client
    while (0 == 0)
    {
        char buffer[1024] = {0};
        int r = recv(cfd, buffer, sizeof(buffer), 0);
        if (r >= 0)
        {
            // buffer: Chứa GET Request của client gửi lên
            char GET[8] = {0};
            char PATH[1024] = {0}; // Đường dẫn mà client muốn lấy (Ex: /Application)
            // Tách nội dung GET Request client gửi lên
            sscanf(buffer, "%s%s", GET, PATH);

            Append(&rootPath, (char *)PATH);

            struct dirent **output = NULL;
            char *html = NULL;
            Append(&html, "<html>");

            int n = scandir(rootPath, &output, NULL, Compare);

            if (n > 0)
            {
                for (int i = 0; i < n; i++)
                {
                    char line[1024] = {0};
                    if (output[i]->d_type == DT_DIR)
                        sprintf(line, "<a href= \"%s\"><b>%s</b></a>)", output[i]->d_name, output[i]->d_name);
                    if (output[i]->d_type == DT_REG)
                        sprintf(line, "<a href= \"%s\"><i>%s</i></a>)", output[i]->d_name, output[i]->d_name);
                    Append(&html, line);
                    Append(&html, "<br>");
                }
                for (int i = 0; i < n; i++)
                {
                    free(output[i]);
                    output[i] = NULL;
                }
                free(output);
                output = NULL;
            }
            else
            {
                Append(&html, (char *)"EMPTY FOLDER<br>");
            }

            Append(&html, "</html>");
            send(cfd, html, strlen(html), 0);
            free(html);
            html = NULL;
        }
        else
        {
            // Neu kết nối bị đóng lại
            break;
        }
    }
    close(cfd);
    free(rootPath);
    rootPath = NULL;
    return NULL;
}

int main()
{
    pthread_t pid;
    sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    unsigned int clen = sizeof(caddr);
    saddr.sin_port = htons(8888);
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = 0;
    int error = bind(sfd, (SOCKADDR *)&saddr, sizeof(saddr));
    // Neu khong loi
    if (error == 0)
    {

        listen(sfd, 10);

        while (0 == 0)
        {
            int tmp = accept(sfd, (SOCKADDR *)&caddr, &clen);
            g_cfd[g_count++] = tmp;
            int *arg = (int *)calloc(1, sizeof(int));
            *arg = tmp;
            pthread_create(&pid, NULL, ClientThread, (void *)arg);
        }
    }
    else
    {
        printf("PORT is already in used\n");
        close(sfd);
    }
}