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
 * @file HTTPFileServer.c
 * @brief Đề bài
 * 1. Bổ sung giao diện upload file vào HTTP File Server
 * Lấy mã http file server từ trên Teams
 * Bổ sung thêm code để ở trang hiện nội dung mỗi thư mục sẽ có thêm giao diện để user
 * Chọn một file từ thiết bị
 * Nhấn nút upload/submit để trình duyệt tự động gửi file đó về server dưới dạng POST
 * Ví dụ về giao diện như hình dưới
 * Tham khảo mã HTML của một form upload file trên Google
 *
 * 2. Lấy code http post xuống và bổ sung code để:
 * Tách tên file từ dữ liệu post và tạo file với tên đó để ghi dữ liệu nhận được
 * Trả lời trình duyệt là đã upload thành công sau khi nhận file
 *
 * @brief Cách chạy chương trình
 * `./HTTPFileServer`: Khởi động server (listen ở port 8888)
 * Mở Browser, gõ `http://localhost:8888` -> hiển thị ra cấu trúc thư mục root
 * Ấn vào từng thư mục để navigate giữa các thư mục
 * Ấn vào các định dạng file để đọc (hoặc download nếu ko đc định nghĩa đuôi file trong đuôi)
 * Chọn file rồi ấn upload -> Upload file vào thư mục '/week10' (thư mục hiện tại)
 *
 * @return int
 *
 * Hàm findpattern(): Tìm pattern -> Trả về vị trí của pattern cần tìm trong mảng 'data',
 * tổng kích thước 'size', vị trí bắt đầu 'offset' (hoặc trả về null nếu ko tìm thấy)
 * - 4: Trừ đi '\r\n' -> Trỏ vào bytes cuối cùng của pattern
 */

#define MAX_CONN_NUM 1024
#define INVALID_SOCKET -1
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
#define MAX_CLIENT 1024
int g_cfd[MAX_CLIENT] = {0};
int g_count = 0;
int sfd = 0;

void Append(char **pdst, const char *src)
{
    char *dst = *pdst; // Xau ky tu cu
    int oldLen = (dst == NULL ? 0 : strlen(dst));
    int newLen = oldLen + strlen(src) + 1;
    dst = (char *)realloc(dst, newLen);
    memset(dst + oldLen, 0, strlen(src) + 1);
    sprintf(dst + oldLen, "%s", src);
    *pdst = dst;
}

int Compare(const struct dirent **a, const struct dirent **b)
{
    if ((*a)->d_type == (*b)->d_type)
        return 0;
    else if ((*a)->d_type == DT_DIR)
        return -1;
    else
        return 1;
}

const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return "";
    return dot + 1;
}

char *findpattern(char *data, int size, int offset, char *pattern)
{
    int i = offset;
    while (i < size)
    {
        if (data[i] == pattern[0])
        {
            int j = i;
            while (j < size && j - i < strlen(pattern) && data[j] == pattern[j - i])
            {
                j++;
            }
            if (j - i == strlen(pattern))
            {
                return data + i;
            }
        }
        i += 1;
    }
    return NULL;
}

void *ClientThread(void *arg)
{
    int cfd = *((int *)arg);
    free(arg);
    arg = NULL;
    char buffer[1024] = {0};

    int r = recv(cfd, buffer, 3, 0);

    if (r > 0)
    {
        char *rootPath = (char *)calloc(7, sizeof(char));
        strcpy(rootPath, "");
        char GET[8] = {0};
        char PATH[1024] = {0};
        if (strcmp(buffer, "GET") == 0)
        {
            recv(cfd, buffer + strlen(buffer), sizeof(buffer) - 3, 0);
            sscanf(buffer, "%s%s", GET, PATH);
            while (strstr(PATH, "%20") != NULL)
            {
                char *tmp = strstr(PATH, "%20");
                tmp[0] = ' ';
                strcpy(tmp + 1, tmp + 3);
            }
            Append(&rootPath, (char *)PATH);
            if (PATH[strlen(PATH) - 1] == '/')
            {
                char *ok = "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: text/html\r\n\r\n";
                struct dirent **output = NULL;
                char *html = NULL;
                Append(&html, "<html>");
                int n = scandir(rootPath, &output, NULL, Compare);
                if (n > 0)
                {
                    for (int i = 0; i < n; i++)
                    {
                        char line[2048] = {0};
                        if (output[i]->d_type == DT_DIR)
                        {
                            if (PATH[strlen(PATH) - 1] == '/')
                                sprintf(line, "<a href = \"%s%s/\"><b>%s</b></a>", PATH, output[i]->d_name, output[i]->d_name);
                            else
                                sprintf(line, "<a href = \"%s/%s/\"><b>%s</b></a>", PATH, output[i]->d_name, output[i]->d_name);
                        }
                        if (output[i]->d_type == DT_REG)
                        {
                            if (PATH[strlen(PATH) - 1] == '/')
                                sprintf(line, "<a href = \"%s%s\"><i>%s</i></a>", PATH, output[i]->d_name, output[i]->d_name);
                            else
                                sprintf(line, "<a href = \"%s/%s\"><i>%s</i></a>", PATH, output[i]->d_name, output[i]->d_name);
                        }
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
                    Append(&html, (char *)"EMPTY FOLDER<br>");

                Append(&html, "<form method=\"post\" enctype=\"multipart/form-data\"><label for=\"myfile\">SELECT FILE TO UPLOAD</label><br><input type=\"file\" id=\"myfile\" name=\"myfile\"><br><input type=\"submit\"></form>");
                Append(&html, "</html>");
                char header[1024] = {0};
                sprintf(header, ok, strlen(html));
                send(cfd, header, strlen(header), 0);
                send(cfd, html, strlen(html), 0);
                free(html);
                html = NULL;
            }
            else
            {
                char *ok = NULL;
                if (strcmp(get_filename_ext(rootPath), "c") == 0)
                {
                    ok = "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: text/plain\r\n\r\n";
                }
                else if (strcmp(get_filename_ext(rootPath), "png") == 0)
                {
                    ok = "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: image/png\r\n\r\n";
                }
                else if (strcmp(get_filename_ext(rootPath), "jpeg") == 0)
                {
                    ok = "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: image/png\r\n\r\n";
                }
                else if (strcmp(get_filename_ext(rootPath), "pdf") == 0)
                {
                    ok = "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: application/pdf\r\n\r\n";
                }
                else if (strcmp(get_filename_ext(rootPath), "mp4") == 0)
                {
                    ok = "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: video/mp4\r\n\r\n";
                }
                else
                    ok = "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: application/octet-stream\r\n\r\n";

                FILE *f = fopen(rootPath, "rb");
                if (f != NULL)
                {
                    fseek(f, 0, SEEK_END);
                    int fsize = ftell(f);
                    fseek(f, 0, SEEK_SET);
                    char header[1024] = {0};
                    sprintf(header, ok, fsize);
                    send(cfd, header, strlen(header), 0);
                    char *data = (char *)calloc(fsize, 1);
                    fread(data, 1, fsize, f);
                    int sent = 0;
                    while (sent < fsize)
                    {
                        int s = send(cfd, data + sent, fsize - sent, 0);
                        if (s <= 0)
                            break;
                        else
                            sent += s;
                    }
                    fclose(f);
                    free(data);
                    data = NULL;
                }
                else
                {
                    if (strstr(rootPath, "favicon"))
                    {
                        char *ok = "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: application/octet-stream\r\n\r\n";
                        FILE *f = fopen("/mnt/e/WSL/favicon.ico", "rb");
                        if (f != NULL)
                        {
                            fseek(f, 0, SEEK_END);
                            int fsize = ftell(f);
                            fseek(f, 0, SEEK_SET);
                            char header[1024] = {0};
                            sprintf(header, ok, fsize);
                            send(cfd, header, strlen(header), 0);
                            char *data = (char *)calloc(fsize, 1);
                            fread(data, 1, fsize, f);
                            send(cfd, data, fsize, 0);
                            fclose(f);
                            free(data);
                            data = NULL;
                        }
                    }
                    else
                    {
                        char *ok = "HTTP/1.1 404 NOT FOUND\r\nContent-Length: 0\r\nContent-Type: text/html\r\n\r\n";
                        send(cfd, ok, strlen(ok), 0);
                    }
                }
            }
        }
        if (strcmp(buffer, "POS") == 0)
        {
            char *header = NULL;
            int header_size = 0;
            char tmpchar;

            // Đọc Header đến khi nào gặp '\r\n\r\n' (vì Header kết thúc bằng ký hiệu đó)
            do
            {
                r = recv(cfd, &tmpchar, 1, 0);
                header = (char *)realloc(header, header_size + 2);
                header[header_size] = tmpchar;
                header[header_size + 1] = 0;
                header_size += 1;
            } while (strstr(header, "\r\n\r\n") == NULL);

            FILE *f = fopen("header.text", "wb");
            fwrite(header, 1, header_size, f);
            fclose(f);

            int content_length = 0;
            // Tách ra 'Content-Length' để biết cần nhận bn bytes dữ liệu
            char *ctl = strstr(header, "Content-Length: ") + strlen("Content-Length: ");
            sscanf(ctl, "%d", &content_length);
            // boundary: Đầu và cuối file đều có
            char *bdr = strstr(header, "boundary=") + strlen("boundary=");
            char boundary[1024] = {0};
            sscanf(bdr, "%s", boundary);

            // Cấp phát thêm bộ nhớ
            char *full_data = (char *)calloc(content_length, 1);
            int full_size = 0;
            while (full_size < content_length)
            {
                // Tiếp tục đọc dữ liệu
                r = recv(cfd, buffer, sizeof(buffer), 0);
                // Nếu hết dữ liệu thì break, còn dữ liệu thì đọc
                if (r > 0)
                {
                    memcpy(full_data + full_size, buffer, r);
                    full_size += r;
                }
                else
                    break;
            }

            f = fopen("postbody.dat", "wb");
            fwrite(full_data, 1, full_size, f);
            fclose(f);

            char *filedata = strstr(full_data, "\r\n\r\n") + 4;
            int offset = filedata - full_data;
            int item = 0;

            do
            {
                char *pattern = findpattern(full_data, full_size, offset, boundary) - 4;
                char filename[1024] = {0};
                sprintf(filename, "tmp%d.dat", item++);
                f = fopen(filename, "wb");
                int filesize = pattern - filedata;
                fwrite(filedata, 1, filesize, f);
                fclose(f);

                filedata = strstr(pattern, "\r\n\r\n");
                if (filedata != NULL)
                {
                    filedata += 4;
                    offset = filedata - full_data;
                }
                else
                    break;
            } while (0 == 0);

            free(full_data);
            free(header);
        }
        free(rootPath);
        rootPath = NULL;
    }

    close(cfd);
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