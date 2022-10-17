#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

/**
 * @brief Đề bài
 * Dùng lệnh scandir để quét tất cả các thư mục con và file của một thư mục hiện thời.
 * Tạo ra chuỗi html kết quả theo định dạng sau:
 * <html>
 * <b>FolderA</b>
 * <b>FolderB</b>
 * <i>FileName1</i>
 * <i>FileName2</i>
 * </html>

 * Thư mục hiện thời làm việc được lưu trong một biến char* (ví dụ lúc khởi tạo ban đầu là "/")
 */

int main()
{
    struct dirent **output = NULL;
    int n = scandir("/", &output, NULL, NULL); // Scanf through all directories inside the root folder

    char outputStr[1024];
    char concatStr[1024];
    sprintf(outputStr, "<html>\n");
    if (output != NULL)
    {
        for (int i = 0; i < n; i++)
        {
            // printf("%s\n", output[i]->d_name);
            if (output[i]->d_type == 4)
            {
                sprintf(concatStr, "<b>%s\n", output[i]->d_name);
            }
            else if (output[i]->d_type == 10)
            {
                sprintf(concatStr, "<i>%s\n", output[i]->d_name);
            }
            strcat(outputStr, concatStr);
        }
    }
    strcat(outputStr, "\n</html>");
    printf("%s\n", outputStr);
}