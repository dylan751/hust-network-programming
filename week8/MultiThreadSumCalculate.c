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

/**
 * @file MultiThreadSumCalculate.c
 * @brief Đề bài
 * White a multi-thread program to compute sum of integers from 1 to K
 * Input the number of threads (N < K)
 * Create N threads
 * Devide the range [1, K] into N segments equally
 * Thread i compute the sum of segment i and add the result to the final sum
 * Print out the final sum
 * @return int
 */

int sum = 0;
int m = 0;
int n = 0;
int k = 0;
pthread_mutex_t *pmutex = NULL;

void *my_last(void *my_arg)
{
    int start = *(int *)my_arg;
    // Tính tổng
    for (int i = start; i <= k; i++)
    {
        // Notes: Nếu có đoạn code có thể bị xung đột -> Cần đồng bộ -> DÙNG MUTEX, tuyệt đối ko dùng biến cờ tự đặt
        // Có thể nhiều thread xung đột -> 1 ông vào thì khoá lại
        pthread_mutex_lock(pmutex);
        sum += i;
        // Sau khi thực hiện lệnh sum += i xong -> Mở khoá
        // Đảm bảo tại 1 thời điểm chỉ có 1 thread chạy lệnh sum += i)
        pthread_mutex_unlock(pmutex);
    }
    free((int *)my_arg);
    return NULL;
}

void *my_func(void *my_arg)
{
    int start = *(int *)my_arg;
    // Tính tổng
    for (int i = start; i < start + m && i <= k; i++)
    {
        // Có thể nhiều thread xung đột -> 1 ông vào thì khoá lại
        pthread_mutex_lock(pmutex);
        sum += i;
        // Sau khi thực hiện lệnh sum += i xong -> Mở khoá
        // Đảm bảo tại 1 thời điểm chỉ có 1 thread chạy lệnh sum += i)
        pthread_mutex_unlock(pmutex);
    }
    free((int *)my_arg);
    return NULL;
}

int main()
{
    // Khởi tạo 1 cơ chế khoá: Mutex
    pmutex = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
    pthread_mutex_init(pmutex, NULL);

    printf("n, k: ");
    scanf("%d%d", &n, &k);

    // Số phần tử của 1 đoạn
    m = k / n;
    // Để lưu lại các thread id
    pthread_t *pid = (pthread_t *)calloc(n + 1, sizeof(pthread_t));
    int count = 0;

    for (int i = 0; i < n; i++)
    {
        // Vị trí bắt đầu
        /*
         Notes: Phải calloc (khai báo động) biến start: vì nếu khai báo tĩnh cục bộ thì sau mỗi
         vòng for, biến có thể bị giải phóng (trong khi thread chưa gọi xong)
        */
        int *start = (int *)calloc(1, sizeof(int));
        *start = i * m + 1;
        pthread_create(&pid[i], NULL, my_func, start);
        count += m;
    }
    // Phần sót lại
    if (count < k)
    {
        count += 1;
        int *start = (int *)calloc(1, sizeof(int));
        *start = count;
        pthread_create(&pid[n], NULL, my_last, start);
    }

    // Đợi các thread my_func xong mới tiếp tục chương trìnhk
    for (int i = 0; i < n; i++)
    {
        pthread_join(pid[i], NULL);
    }

    free(pid);

    printf("sum = %d\n", sum);
    pthread_mutex_destroy(pmutex);
}