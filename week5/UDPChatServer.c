#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

/**
 * @file UDPChatServer.c
 * @brief Đề bài
 * Tạo 1 UDP Server để lắng nghe ở port 5000 và 6000
 * Nhận gói tin đăng ký ở cổng 5000, khi nhận thì lưu lại địa chỉ của client gửi đến vào danh sách
 * Nhận gói tin chat ở cổng 6000, khi nhận được thì gửi lại cho tất cả các client đã đăng ký (cổng 7000)
 *
 * @brief Cách chạy chương trình
 * `./UDPChatServer`: Khởi động server (listen ở port 5000 và 6000)
 * Dùng Netcat để làm client test cho server này.
 * nc -vv -u <địa chỉ server> 5000 (để gửi gói đăng ký xong rồi thoát luôn Ctrl+C)
 * nc -vv -u <địa chỉ server> 6000 (để gửi gói tin chat)
 * nc -vv -u -l -p 7000 (để nhận nội dung chat của các client khác mà server gửi)
 * Lệnh: `netstat -anvp tcp | awk 'NR<3 || /LISTEN/'`: Liệt kê các cổng đang chạy trên máy
 */

int main()
{
}