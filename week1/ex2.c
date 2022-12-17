#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Đề bài
 * Nhập từng số đến khi nhập 0
 * Cấp phát bộ nhớ -> nhét phần tử đó vào mảng
 * In ra mảng đó
 */
int main()
{
    float tmp;
    int dem = 0;
    float *x = NULL;

    printf("Input numbers (exit when you input 0): ");
    do
    {
        scanf("%f", &tmp);
        if (tmp != 0)
        {
            x = (float *)realloc(x, (dem + 1) * sizeof(float)); // realloc(Vùng nhớ cũ, thêm bao nhiêu vùng nhớ)
            x[dem] = tmp;
            dem += 1;
        }
    } while (tmp != 0);

    for (int i = 0; i < dem; i++)
    {
        printf("%.2f\n", x[i]);
    }

    free(x);
}