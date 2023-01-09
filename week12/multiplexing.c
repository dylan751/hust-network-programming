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

#define MAX_CONN_NUM 1024
#define INVALID_SOCKET -1
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

int main()
{
    // Khai báo socket lễ tân
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    unsigned int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5000);
    saddr.sin_addr.s_addr = 0;

    bind(s, (SOCKADDR *)&saddr, sizeof(saddr));
    listen(s, 10);

    fd_set *fdread;
    while (0 == 0)
    {
        // Làm đồng thời việc accept và nhận dữ liệu -> dùng thăm dò
        // Xoá sạch tập này đi (để ko bị lẫn vs các lần lặp)
        FD_ZERO(&fdread);
        // Thêm các tập thăm dò
        FD_SET(s, &fdread); // Thêm socket s vào tập read

        // Thêm cả các socket đã kết nối vào fdread

        select(FD_SETSIZE, &fdread, NULL, NULL, NULL); // Tạm thời chỉ sử dụng tập read -> các tập khác để null

        if (FD_ISSET(s, &fdread))
        {

            // Chương trình treo ở select() cho đến khi có kết nối tới
            // Gọi accept sau khi đã có kết nối -> hàm accept sẽ ko bị treo nữa
            int tmp = accept(s, (SOCKADDR *)&caddr, &clen);
            printf("Accepted another client\n");
            // Lưu tmp lại để SELECT lần sau
        }

        // Lặp để kiểm tra các socket khác
    }
}