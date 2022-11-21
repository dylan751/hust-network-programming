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

#define INVALID_SOCKET -1
#define MAX_CLIENT 1024
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

void sighandler(int signum)
{
    int stat = 0;
    while (waitpid(-1, &stat, WNOHANG) > 0);
}

void udp_process()
{
    char* response = "127.0.0.1"; //KHONG QUAN TRONG VI CLIENT CO THE TACH IP TU RECVFROM
    char buffer[1024];
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKADDR_IN saddr, caddr;
    unsigned int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(4000);
    saddr.sin_addr.s_addr = 0;
    bind(fd, (SOCKADDR*)&saddr, sizeof(saddr));
    while (0 == 0)
    {
        memset(buffer, 0, sizeof(buffer));
        recvfrom(fd, buffer, sizeof(buffer), 0, (SOCKADDR*)&caddr, &clen);
        while ( buffer[strlen(buffer) - 1] == '\r' ||
                buffer[strlen(buffer) - 1] == '\n')
        {
            buffer[strlen(buffer) - 1] = 0;    
        }
        caddr.sin_port = htons(7000);
        sendto(fd, response, strlen(response), 0, (SOCKADDR*)&caddr, clen);
        FILE* f = fopen("clients.txt","a+t");
        fprintf(f, "%s %s\n", inet_ntoa(caddr.sin_addr), buffer);
        fclose(f);
    }
}

void tcp_process()
{
    char buffer[1024];
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    unsigned int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5000);
    saddr.sin_addr.s_addr = 0;
    bind(fd, (SOCKADDR*)&saddr, sizeof(saddr));
    listen(fd, 10);
    while (0 == 0)
    {
        int cfd = accept(fd, (SOCKADDR*)&caddr, &clen);
        FILE* f = fopen("clients.txt", "rt");
        while (!feof(f))
        {
            char buffer[1024] = { 0 };
            fgets(buffer, sizeof(buffer), f);
            send(cfd, buffer, strlen(buffer), 0);
        }
        fclose(f);
        send(cfd, "END", 3, 0);
        close(cfd);
    }
}

int main()
{
    signal(SIGCHLD, sighandler);
    FILE* f = fopen("clients.txt","wt");
    fclose(f);
    if (fork() == 0)
    {
        udp_process();
    }
    if (fork() == 0)
    {
        tcp_process();
    }
    while (0 == 0)
    {
        sleep(1000);
    }
}