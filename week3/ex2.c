#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/**
 * @file ex2.c
 * @brief Đề bài: Get host's IP address by name
 * Đầu vào: domain name
 * Đầu ra: IP address
 * 
 * Example input: ./ex2 google.com
 * Example output: 142.250.66.46 (An IP Address in ASCII)
 *
 * Nối socket vào thử IP đó
 */

#define INVALID_SOCKET -1

int main(int argc, char **argv)
{
    // if (argc > 1)
    {
        // char *domain = argv[1];
        char *domain = "vnexpress.net";
        struct hostent *presult = gethostbyname(domain); // presult: Pointer Result
        if (presult != NULL)
        {
            struct hostent host = *presult;
            int count = 0;
            int type = host.h_addrtype;
            struct in_addr inaddr;
            while (host.h_addr_list[count] != NULL)
            {
                char *addr = host.h_addr_list[count]; // 4 bytes địa chỉ IP
                memcpy(&inaddr, addr, sizeof(inaddr));
                // Chuyển đổi addr -> ASCII cho dễ đọc
                char *saddr = inet_ntoa(inaddr);
                printf("%s\n", saddr);
                count += 1;
            }

            if(count > 0) {
                // Tạo 1 socket mới
                int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if(s != INVALID_SOCKET) {
                    struct sockaddr_in addr;
                    addr.sin_family = AF_INET;
                    addr.sin_addr = inaddr;
                    addr.sin_port = htons(80); // htons: Host To Net Short
                    // Kết nối
                    int error = connect(s, (struct sockaddr*)&addr, sizeof(addr));
                    if(error == 0) // Nếu connect thành công
                    {
                        char buffer[1024] = {0};
                        // Gửi message "Hello" cho trang web
                        // Vì message không đúng chuẩn nào -> Web sẽ báo lại lỗi "Bad Request" -> In ra
                        char* hello = "Hello";
                        int sent = send(s, hello, strlen(hello), 0);
                        int received = recv(s, buffer, sizeof(buffer), 0);
                        printf("%d %s\n", received, buffer);
                    }
                }
            }
        }
    }
}
