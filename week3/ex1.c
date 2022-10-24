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
 * Nếu người dùng ấn vào folder -> Đi vào trong folder đấy
 */

// Hàm nối vào cuối xâu (nối thêm xấu `src` vào sau xâu `dst`)
// pdst: Pointer Destination
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

// So sánh 2 cấu trúc thư mục (là kiểu DIR hay FILE)
int Compare(const struct dirent **a, const struct dirent **b)
{
    if ((*a)->d_type == (*b)->d_type)
        return 0;
    else if ((*a)->d_type == DT_DIR)
        return -1;
    else
        return 1;
}

int main()
{
    char *rootPath = "/";
    char *tmpPath = NULL;
    tmpPath = (char *)calloc(strlen(rootPath) + 1, 1);
    strcpy(tmpPath, rootPath);

    int exit = 0;

    // Lặp lại thao tác nếu người dùng ấn vào 1 folder -> đi vào folder đó (hoặc đi ra)
    do
    {
        struct dirent **output = NULL;
        char *html = NULL;
        Append(&html, "<html>");

        int n = scandir(tmpPath, &output, NULL, Compare);

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
        }
        Append(&html, "</html>");
        printf("%s", html);

        // Write html output to file
        FILE *f = fopen("output.html", "wt");
        fprintf(f, "%s", html);
        fclose(f);

        free(html);
        html = NULL;

        char command[1024] = {0};
        fgets(command, sizeof(command), stdin); // Đọc input người dùng vào command
        while (command[strlen(command) - 1] == '\r' || command[strlen(command) - 1] == '\n')
        {
            command[strlen(command) - 1] = 0; // Cắt byte Enter ở cuối command đi
        }

        exit = 1;
        // Ấn vào ".." -> Lùi lên 1 cấp
        if (strcmp(command, "..") == 0)
        {
            exit = 0;
            if (strcmp(tmpPath, rootPath) != 0) // Không phải root path
            {
                tmpPath[strlen(tmpPath) - 1] = 0; // Cắt dấu "/" ở cuối đi
                while (tmpPath[strlen(tmpPath) - 1] != '/')
                {
                    tmpPath[strlen(tmpPath) - 1] = 0; // Cắt dấu "/" ở cuối đi cho đến root path
                }
            }
        }
        else
        {
            for (int i = 0; i <= n; i++)
            {
                //Ấn vào tên folder -> Tiến vào 1 cấp
                if (output[i]->d_type == DT_DIR && strcmp(output[i]->d_name, command) == 0)
                {
                    tmpPath = (char *)realloc(tmpPath, strlen(tmpPath) + strlen(command) + 1);
                    sprintf(tmpPath + strlen(tmpPath), "%s/", command);
                    exit = 0;
                    break;
                }
            }
        }

        for (int i = 0; i < n; i++)
        {
            free(output[i]);
            output[i] = NULL;
        }
        free(output);
        output = NULL;
    } while (!exit);
}
