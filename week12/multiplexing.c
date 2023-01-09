/**
 * @brief Multiplexing: Hàm select()
 *
 * @brief Ý nghĩa các tham số
 * (số lượng file descriptor max, tập các socket (file descriptor) mà cta muốn thăm dò sự kiện liên quan đến read (dữ liệu đi về phía cta), tập các socket (file descriptor) mà cta muốn thăm dò sự kiện liên quan đến write, thăm dò các lỗi xảy ra trên file descriptor, chỉ định timeout)
 * Tập nào mà không muốn thăm dò thì để null (vdu: tập write để null để chỉ thăm dò các dữ liệu đến)
 *
 * @brief  Useful macros
 * FD_ZERO: Dùng để xoá sạch các tập đi
 * FD_SET: Đưa 1 socket fd vào trong 1 tập
 * FD_CLR: Bỏ 1 socket fd ra khỏi 1 tập
 * FD_ISSET: Kiểm tra xem socket fd có tồn tại trong tập ko
 *
 */

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
 * @file multiplexing.c
 * @brief Đề bài
 * Viết 1 Server để:
 * Nhận kết nối và lưu các socket vào một mảng
 * Nhận dữ liệu từ một client
 * In dữ liệu nhận được ra màn hình
 * Gửi dữ liệu đó đến tất cả các client còn lại (không gửi lại cho chính client vừa gửi)
 *
 * @brief Cách chạy chương trình
 * `./multiplexingWithThread`: Khởi động server (listen ở port 8888)
 * Mở 1 terminal mới, gõ lệnh: `nc -vv 127.0.0.1 8888` để Kết nối tới host: 127.0.0.1, port: 8888 (client 1)
 * Mở 1 terminal mới, gõ lệnh: `nc -vv 127.0.0.1 8888` để Kết nối tới host: 127.0.0.1, port: 8888 (client 2)
 * Mở 1 terminal mới, gõ lệnh: `nc -vv 127.0.0.1 8888` để Kết nối tới host: 127.0.0.1, port: 8888 (client 3)
 * Lệnh: `lsof -PiTCP -sTCP:LISTEN`: Liệt kê các cổng đang chạy trên máy
 * Dùng terminal của client 1, gõ chữ gì thì sẽ hiển thị trên terminal của tất cả các terminal của các client khác
 */

#define MAX_CLIENT 1024
#define MAX_CONN_NUM 1024
#define INVALID_SOCKET -1
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
int g_client[MAX_CLIENT];
int g_count = 0;

int main()
{
    // Khai báo socket lễ tân
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    unsigned int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.s_addr = 0;

    bind(s, (SOCKADDR *)&saddr, sizeof(saddr));
    listen(s, 10);

    fd_set fdread;
    while (0 == 0)
    {
        // Làm đồng thời việc accept và nhận dữ liệu -> dùng thăm dò
        // Xoá sạch tập này đi (để ko bị lẫn vs các lần lặp)
        FD_ZERO(&fdread);
        // Thêm các tập thăm dò
        FD_SET(s, &fdread); // Thêm socket s vào tập read

        // Thêm cả các socket đã kết nối vào fdread
        for (int i = 0; i < g_count; i++)
        {
            FD_SET(g_client[i], &fdread); // Thêm socket
        }

        select(FD_SETSIZE, &fdread, NULL, NULL, NULL); // Tạm thời chỉ sử dụng tập read -> các tập khác để null

        if (FD_ISSET(s, &fdread))
        {
            // Chương trình treo ở select() cho đến khi có kết nối tới
            // Gọi accept sau khi đã có kết nối -> hàm accept sẽ ko bị treo nữa
            int tmp = accept(s, (SOCKADDR *)&caddr, &clen);
            printf("Accepted another client\n");
            // Lưu tmp lại để SELECT lần sau
            g_client[g_count++] = tmp;
        }

        // Lặp để kiểm tra các socket khác
        for (int i = 0; i < g_count; i++)
        {
            // Nếu có dữ liệu kết nối đến
            if (FD_ISSET(g_client[i], &fdread))
            {
                char buffer[1024] = {0};
                recv(g_client[i], buffer, sizeof(buffer), 0);

                // Gửi buffer tới tất cả client khác
                for (int j = 0; j < g_count; j++)
                {
                    if (g_client[j] != g_client[i])
                    {
                        send(g_client[j], buffer, strlen(buffer), 0);
                    }
                }
            }
        }
    }
}