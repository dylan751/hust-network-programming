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
char name[1024];
int hasServerAdress = 0;
int tcp_process_id = 0;

void sighandler(int signum)
{
    if (signum == SIGCHLD)
    {
        int stat;
        while (waitpid(-1, &stat, WNOHANG) > 0)
            ;
    }
    if (signum == SIGUSR1)
    {
        hasServerAdress = 1;
    }
}

void udp_process(char *name)
{
    printf("TCP Process ID: %d\n", tcp_process_id);
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKADDR_IN saddr, baddr, caddr;
    unsigned int clen = sizeof(caddr);
    baddr.sin_family = AF_INET;
    baddr.sin_port = htons(4000);
    baddr.sin_addr.s_addr = inet_addr("10.70.255.255"); //

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(7000);
    saddr.sin_addr.s_addr = 0;
    bind(s, (SOCKADDR *)&saddr, sizeof(saddr));
    int on = 1;
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
    printf("Broadcast client name to server\n");
    sendto(s, name, strlen(name), 0, (SOCKADDR *)&baddr, sizeof(baddr));
    char buffer[1024] = {0};
    int r = recvfrom(s, buffer, sizeof(buffer), 0, (SOCKADDR *)&caddr, &clen);
    printf("Got %d bytes from server: %s\n", r, buffer);
    if (r > 0)
    {
        FILE *f = fopen("saddr.bin", "wb");
        fwrite(&caddr, 1, sizeof(caddr), f);
        fclose(f);
        kill(tcp_process_id, SIGUSR1);
    }
    close(s);
}

void tcp_process()
{
    printf("Waiting for server adress...\n");
    while (!hasServerAdress)
    {
        sleep(100);
    }
    printf("Connecting to server...\n");
    SOCKADDR_IN serveraddr;
    FILE *f = fopen("saddr.bin", "rb");
    fread(&serveraddr, 1, sizeof(serveraddr), f);
    fclose(f);
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5000);
    saddr.sin_addr.s_addr = serveraddr.sin_addr.s_addr;
    connect(fd, (SOCKADDR *)&saddr, sizeof(saddr));
    char peers[1024][1024];
    int peerCount = 0;
    while (0 == 0)
    {
        char buffer[1024] = {0};
        int n = recv(fd, buffer, sizeof(buffer) - 1, 0);
        printf("%s\n", buffer);
        if (strcmp(buffer, "END") == 0)
        {
            break;
        }
        else
        {
            sscanf(buffer, "%s", peers[peerCount]);
            peerCount += 1;
        }
    }
    // NHAN INPUT TU BAN PHIM DE GUI
    char fname[1024] = {0};
    int index = 0;
    scanf("%s%d", fname, &index);

    // DOC FILE FNAME GUI DEN peers[index]
    // NHU VAY CLIENT PHAI CO THEM 1 CHILD PROCESS DE NHAN DU LIEU TCP/8888
}

void file_recv_process()
{
    // LIEN TUC DOI TCP 8888
    // ACCEPT -> RECV
}

int main()
{
    signal(SIGUSR1, sighandler);
    signal(SIGCHLD, sighandler);

    memset(name, 0, sizeof(name));
    fgets(name, sizeof(name), stdin);

    if ((tcp_process_id = fork()) == 0)
    {
        tcp_process();
    }
    else
    {
        if (fork() == 0)
        {
            udp_process(name);
        }
        else
        {
            if (fork() == 0)
            {
                file_recv_process();
            }
        }
    }

    while (1)
    {
        sleep(1000);
    }
}