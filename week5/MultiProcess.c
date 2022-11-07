#include <stdio.h>
#include <unistd.h> // For fork()

int main()
{
    /*
    // Nhân đôi tiến trình
    fork();
    // Nhân đôi tiến trình trên cả tiến trình con và tiến trình cha
    fork();
    printf("Hello World!\n"); // -> Sẽ in ra 4 chữ Hello World
    */

    // Cách kiểm soát việc nhân bản tiến trình con (Luôn check điều kiện các tiến trình)
    // Lệnh fork(): Trả về 0 nếu là tiến trình cha, trả về id khác nếu là tiến trình con
    if (fork() == 0)
    {
        printf("A child has been created!\n");
    }
    else
    {
        if (fork() == 0)
        {

            printf("A child has been created!\n");
        }
        else
        {
            fork();
            printf("A child has been created!\n");
        }
    }

    printf("Hello\n");
    return 0;
}