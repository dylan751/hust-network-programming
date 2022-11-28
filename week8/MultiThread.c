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
    printf("Hello from my_func\n");
    return NULL;
}

int main()
{
    pthread_t pid;
    void* status = NULL;
    // pthread_create(): Tạo 1 thread mới
    pthread_create(&pid, NULL, my_func, NULL); // (Biến kết quả ra, , con trỏ hàm, )

    // Nếu chỉ chạy ntn thì ko thấy in ra "Hello from my_func": 
    // Vì thread đó chưa kịp chạy thì thread main đã kết thúc r
    printf("Hello from main\n"); 

    // 'pthread_join': Đợi thread my_func xong mới tiếp tục chương trình
    pthread_join(pid, &status);
    printf("Thread %d completed\n", (int)pid); 
}