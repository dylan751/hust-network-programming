#include <stdio.h>

/**
 * @brief Đề bài
 * Nhập 1 số float
 * In ra 4 bytes đầu tiên của số đó
 */
int main()
{
    float x = 1.5;
    unsigned char *y = (unsigned char *)&x;
    for (int i = 0; i < 4; i++)
    {

        printf("%X", y[i]);;
    }
    printf("\n");
}