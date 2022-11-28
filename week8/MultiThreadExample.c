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

void *my_func(void *my_arg)
{
    printf("Hello from my_func: %d\n", *(int *)my_arg);
    return NULL;
}

int main()
{
    pthread_t pid1, pid2, pid3, pid4, pid5;
    void *status = NULL;
    int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;

    // pthread_create(): Tạo 1 thread mới
    // (thread_id, ..., con trỏ hàm tới nơi chứa logic của thread, tham số của hàm)
    pthread_create(&pid1, NULL, my_func, &a1);
    pthread_create(&pid2, NULL, my_func, &a2);
    pthread_create(&pid3, NULL, my_func, &a3);
    pthread_create(&pid4, NULL, my_func, &a4);
    pthread_create(&pid5, NULL, my_func, &a5);

    // Nếu chỉ chạy ntn thì ko thấy in ra "Hello from my_func":
    // Vì thread đó chưa kịp chạy thì thread main đã kết thúc r
    printf("Hello from main\n");

    // Đợi các thread my_func xong mới kết thúc chương trình
    getc(stdin);
}