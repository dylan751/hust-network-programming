#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Đề bài
 * Dùng hàm system để thực thi lệnh ifconfig
 * -> định hướng lại kết quả ifconfig vào file (VD: ifconfig.out)
 * Đọc file ifconfig.out
 * -> phân tích và in ra màn hình tất cả các địa chỉ IPv4 của máy có trong kết quả ifconfig
 */

int main()
{
    system("ifconfig > ifconfig.out");

    // Read file
    FILE *f = fopen("ifconfig.out", "rb");

    fseek(f, 0, SEEK_END); // Move the pointer to the end of file
    int fsize = ftell(f);  // Return the number of bytes in the file
    fseek(f, 0, SEEK_SET); // Move the pointer to the start of the file

    char *data = (char *)calloc(fsize + 1, sizeof(char)); // Always allocate +1 bytes for memory (for safety)
    fread(data, sizeof(char), fsize, f);

    fclose(f);

    char *tmp = data;
    char adapterName[1024];
    char inet[1024];
    char ip[1024];

    // Search until find "flags="
    while (strstr(tmp, "flags=") != NULL)
    {
        tmp = strstr(tmp, "flags=");
        while (tmp[0] != '\r' && tmp[0] != '\n' && tmp != data)
        {
            tmp -= 1;
        }

        if (tmp[0] == '\r' || tmp[0] == '\n') // Pass
            tmp += 1;

        // Get the adapter name
        memset(adapterName, 0, sizeof(adapterName)); // Before using string in C -> Must memset for safety (set all chars to 0)
        sscanf(tmp, "%s", adapterName);

        // Get the IPV4
        if (strstr(tmp, "inet ") != NULL)
        {
            tmp = strstr(tmp, "inet ");
            memset(inet, 0, sizeof(inet)); // Initially set all chars to 0
            memset(ip, 0, sizeof(ip));     // Initially set all chars to 0
            sscanf(tmp, "%s%s", inet, ip);
            printf("%s %s\n", adapterName, ip);
        }
        else
        {
            printf("%s IPv4 not available", adapterName);
        }
    }

    free(data);
}
