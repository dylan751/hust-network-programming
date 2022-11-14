#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

/**
 * @file TCPEchoServer.c
 * @brief Đề bài
 * Tiến trình cha: accept để tạo kết nối
 * Tiền trình con:
    * Nhận dữ liệu từ một kết nối (client)
    * Gửi dữ liệu nhận được ngược lại cho tất cả các client đang kết nối với server

 *
 * @brief Cách chạy chương trình
 * 
 */