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
 */

#define MAX_CLIENT 1024
#define MAX_CONN_NUM 1024
#define INVALID_SOCKET -1
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
int g_client[MAX_CLIENT];
int g_count = 0;

void *ClientThread(void *arg)
{
    fd_set fdread;
    int startIndex = *((int *)arg); // Ép arg sang int*, rồi lấy nội dung của nó
    int endIndex = startIndex + N - 1;

    FD_ZERO(&fdread);
    for (int i = startIndex; i <= endIndex && i < g_count; i++)
    {
        // Thêm g_client[i] vào fdread
        FD_SET(g_client[i], &fdread);
    }
    select(FD_SETSIZE, &fdread, NULL, NULL, NULL);

    // Nếu vượt qua hàm `select` -> có client đang gửi dữ liệu đến
    for (int i = startIndex; i <= endIndex && i < g_count; i++)
    {
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
        for (int i = 0; i < g_count && i < N; i++)
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

            // Với g_count 1 -> N, N+1 -> 2N, 2N+1 -> 3N ... thì tạo từng luồng (mỗi luồng có N client)
            if ((g_count > N) && (g_count % N == 1))
            {
                // Tạo luồng tham số của luồng là g_count - 1 = chỉ số của socket bắt đầu
                int *arg = (int *)calloc(1, sizeof(int));
                *arg = g_count - 1;
                pthread_t tid = 0;
                pthread_create(&tid, NULL, ClientThread, arg);
            }
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