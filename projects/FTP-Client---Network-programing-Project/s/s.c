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
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

#define INVALID_SOCKET -1
#define INVALID_IP -1
#define MAX_SIZE 1024
#define PORT 9000
#define DEFAULT_PORT 3000
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

/**
 * Trim whiteshpace and line ending
 * characters from a string
 */
void trimstr(char *str, int n)
{
	int i;
	for (i = 0; i < n; i++) {
		if (isspace(str[i])) str[i] = 0;
		if (str[i] == '\n') str[i] = 0;
	}
}

int socket_create()
{
	int sockfd;
	int yes = 1;
	SOCKADDR_IN sock_addr;

	// create new socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() error"); 
		return -1;
	}

	// set local address info
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(PORT);
	sock_addr.sin_addr.s_addr = INADDR_ANY;

	// bind
	int flag = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag));

	if (bind(sockfd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) {
		close(sockfd);
		perror("bind() error"); 
		return -1;
	}
   
	// begin listening for incoming TCP requests
	if (listen(sockfd, 5) < 0) {
		close(sockfd);
		perror("listen() error");
		return -1;
	}              
	return sockfd;
}

/**
 * Create new socket for incoming client connection request
 * Returns -1 on error, or fd of newly created socket
 */
int socket_accept(int sock_listen)
{
	int sockfd;
	SOCKADDR_IN client_addr;
	int len = sizeof(client_addr);

	// Wait for incoming request, store client info in client_addr
	sockfd = accept(sock_listen, (SOCKADDR*) &client_addr, &len);
	
	if (sockfd < 0) {
		perror("accept() error"); 
		return -1; 
	}
	return sockfd;
}

int send_response(int sockfd, int rc)
{
	int conv = rc;
	if (send(sockfd, &conv, sizeof(conv), 0) < 0 ) {
		perror("error sending...\n");
		return -1;
	}
	return 0;
}

/**
 * Receive data on sockfd
 * Returns -1 on error, number of bytes received 
 * on success
 */
int recv_data(int sockfd, char* buf, int bufsize){
	memset(buf, 0, bufsize);
	int num_bytes = recv(sockfd, buf, bufsize, 0);
	if (num_bytes < 0) {
		return -1;
	}
	return num_bytes;
}

/**
 * Authenticate a user's credentials
 * Return 1 if authenticated, 0 if not
 */
int ftserve_check_user(char*user, char*pass)
{
	char username[MAX_SIZE];
	char password[MAX_SIZE];
	char *pch;
	char buf[MAX_SIZE];
	char *line = NULL;
	size_t num_read;									
	size_t len = 0;
	FILE* fd;
	int auth = 0;
	
	fd = fopen(".auth", "r");
	if (fd == NULL) {
		perror("file not found");
		exit(1);
	}	

	while ((num_read = getline(&line, &len, fd)) != -1) {
		memset(buf, 0, MAX_SIZE);
		strcpy(buf, line);
		
		pch = strtok (buf," ");
		strcpy(username, pch);

		if (pch != NULL) {
			pch = strtok (NULL, " ");
			strcpy(password, pch);
		}

		// remove end of line and whitespace
		trimstr(password, (int)strlen(password));

		if ((strcmp(user,username)==0) && (strcmp(pass,password)==0)) {
			auth = 1;
			break;
		}		
	}
	free(line);	
	fclose(fd);	
	return auth;
}

/** 
 * Log in connected client
 */
int ftserve_login(int sock_control)
{	
	char buf[MAX_SIZE];
	char user[MAX_SIZE];
	char pass[MAX_SIZE];	
	memset(user, 0, MAX_SIZE);
	memset(pass, 0, MAX_SIZE);
	memset(buf, 0, MAX_SIZE);
	
	// Wait to recieve username
	if ( (recv_data(sock_control, buf, sizeof(buf)) ) == -1) {
		perror("recv error\n"); 
		exit(1);
	}	

	strcpy(user,buf+5);// 'USER ' has 5 char

	// tell client we're ready for password
	send_response(sock_control, 331);					
	
	// Wait to recieve password
	memset(buf, 0, MAX_SIZE);
	if ( (recv_data(sock_control, buf, sizeof(buf)) ) == -1) {
		perror("recv error\n"); 
		exit(1);
	}
	
	strcpy(pass,buf+5); // 'PASS ' has 5 char
	
	return (ftserve_check_user(user, pass));
}

/**
 * Wait for command from client and
 * send response
 * Returns response code
 */
int ftserve_recv_cmd(int sock_control, char*cmd, char*arg)
{	
	int rc = 200;
	char user_input[MAX_SIZE];
	
	memset(user_input, 0, MAX_SIZE);
	memset(cmd, 0, 5);
	memset(arg, 0, MAX_SIZE);

	// Wait to recieve command
	if ((recv_data(sock_control, user_input, sizeof(user_input)) ) == -1) {
		perror("recv error\n"); 
		return -1;
	}
	
	strncpy(cmd, user_input, 4);
	strcpy(arg, user_input + 5);
	
	if (strcmp(cmd, "QUIT") == 0) {
		rc = 221;
	} else if((strcmp(cmd, "USER") == 0) || (strcmp(cmd, "PASS") == 0) ||
			  (strcmp(cmd, "LIST") == 0) || (strcmp(cmd, "RETR") == 0) ||
			  (strcmp(cmd, "CWD ") == 0) || (strcmp(cmd, "PWD ") == 0) ||
			  (strcmp(cmd, "STOR") == 0)) {
		rc = 200;
	} else { //invalid command
		rc = 502;
	}

	send_response(sock_control, rc);	
	return rc;
}

/**
 * Connect to remote host at given port
 * Returns:	socket fd on success, -1 on error
 */
int socket_connect(int port, char*host)
{
	int sockfd;  					
	SOCKADDR_IN dest_addr;

	// create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        	perror("error creating socket");
        	return -1;
    }

	// create server address
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = inet_addr(host);

	// Connect on socket
	if(connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0 ) {
        	perror("error connecting to server");
		return -1;
    	}    
	return sockfd;
}

/**
 * Open data connection to client 
 * Returns: socket for data connection
 * or -1 on error
 */
int ftserve_start_data_conn(int sock_control)
{
	char buf[1024];	
	int wait, sock_data;

	// Wait for go-ahead on control conn
	if (recv(sock_control, &wait, sizeof wait, 0) < 0 ) {
		perror("Error while waiting");
		return -1;
	}

	// Get client address
	SOCKADDR_IN client_addr;
	int len = sizeof(client_addr);
	getpeername(sock_control, (struct sockaddr*)&client_addr, &len);
	inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf));

	// Initiate data connection with client
	if ((sock_data = socket_connect(DEFAULT_PORT, buf)) < 0)
		return -1;

	return sock_data;		
}


/**
 * Send list of files in current directory
 * over data connection
 * Return -1 on error, 0 on success
 */
int ftserve_list(int sock_data, int sock_control)
{
	struct dirent **output = NULL;
	char curr_dir[MAX_SIZE], msgToClient[MAX_SIZE];
	memset(curr_dir, 0, MAX_SIZE);
	memset(msgToClient,0,MAX_SIZE);

	getcwd(curr_dir,sizeof(curr_dir));
	int n = scandir(curr_dir, &output, NULL, NULL);
	if (n > 0){
        for (int i = 0; i < n; i++)
        {
        	if(strcmp(output[i]->d_name, ".") != 0 && strcmp(output[i]->d_name, "..") != 0) {
            	strcat(msgToClient,output[i]->d_name);
            	strcat(msgToClient, "  ");
        	}
        }
    }
    strcat(msgToClient, "\n");
	if(send(sock_data, msgToClient,strlen(msgToClient),0) < 0) {
		perror("error");
	}

	return 0;	
}

/**
 * Send list of files in current directory
 * over data connection
 * Return -1 on error, 0 on success
 */
int ftpServer_cwd(int sock_control, char* folderName)
{
	if(chdir(folderName) == 0) //change directory
	{
		send_response(sock_control, 250); // 250 Directory successfully changed.
	}
	else
	{
		send_response(sock_control, 550); //550 Requested action not taken
	}
	return 0;	
}

/**
 * Send list of files in current directory
 * over data connection
 * Return -1 on error, 0 on success
 */
void ftpServer_pwd(int sock_control, int sock_data)
{
	char curr_dir[MAX_SIZE -1], msgToClient[MAX_SIZE];
	memset(curr_dir, 0, MAX_SIZE);
	memset(msgToClient,0,MAX_SIZE);

	getcwd(curr_dir,sizeof(curr_dir));
	sprintf(msgToClient,"%s\n",curr_dir);
	if(send(sock_data, msgToClient, strlen(msgToClient),0) < 0) {
		perror("error");
		send_response(sock_control,550);
	}
	send_response(sock_control, 212);
}

/**
 * Send file specified in filename over data connection, sending
 * control message over control connection
 * Handles case of null or invalid filename
 */
void ftserve_retr(int sock_control, int sock_data, char* filename)
{	
	FILE* fd = NULL;
	char data[MAX_SIZE];
	memset(data, 0, MAX_SIZE);
	size_t num_read;							
		
	fd = fopen(filename, "r");
	
	if (!fd) {	
		// send error code (550 Requested action not taken)
		send_response(sock_control, 550);
		
	} else {	
		// send okay (150 File status okay)
		send_response(sock_control, 150);
	
		do {
			num_read = fread(data, 1, MAX_SIZE, fd);

			if (num_read < 0) {
				printf("error in fread()\n");
			}

			// send block
			if (send(sock_data, data, num_read, 0) < 0)
				perror("error sending file\n");

		} while (num_read > 0);													
			
		// send message: 226: closing conn, file transfer successful
		send_response(sock_control, 226);

		fclose(fd);
	}
}

int recvFile(int sock_control ,int sock_data, char* filename) {
    char data[MAX_SIZE];
    int size, stt = 0;

    recv(sock_control, &stt, sizeof(stt), 0);
    // printf("%d\n", stt);
    if( stt == 550) {
    	printf("can't not open file!\n");
    	return -1;
    } else {

	    FILE* fd = fopen(filename, "w");
	    
	    while ((size = recv(sock_data, data, MAX_SIZE, 0)) > 0) {
	        fwrite(data, 1, size, fd);
	    }

	    if (size < 0) {
	        perror("error\n");
	    }

	    fclose(fd);
	    return 0;
    }
    return 0;
}

/** 
 * Child process handles connection to client
 */
void ftserve_process(int sock_control)
{
	int sock_data;
	char cmd[5];
	char arg[MAX_SIZE];

	// Send welcome message
	send_response(sock_control, 220);

	// Authenticate user
	if (ftserve_login(sock_control) == 1) {
		send_response(sock_control, 230);
	} else {
		send_response(sock_control, 430);	
		exit(0);
	}	
	
	while (1) {
		// Wait for command
		int rc = ftserve_recv_cmd(sock_control, cmd, arg);
		
		if ((rc < 0) || (rc == 221)) {
			break;
		}
		
		if (rc == 200 ) {
			// Open data connection with client
			if ((sock_data = ftserve_start_data_conn(sock_control)) < 0) {
				close(sock_control);
				exit(1); 
			}

			// Execute command
			if (strcmp(cmd, "LIST") == 0) { 			// Do list
				ftserve_list(sock_data, sock_control);
			} else if (strcmp(cmd, "CWD ") == 0) {		// change directory
				ftpServer_cwd(sock_control, arg);
			} else if (strcmp(cmd, "PWD ") == 0) {		// print working directory
				ftpServer_pwd(sock_control, sock_data);
			} else if (strcmp(cmd, "RETR") == 0) {		// RETRIEVE: get file
				ftserve_retr(sock_control, sock_data, arg);
			} else if (strcmp(cmd, "STOR") == 0) {		// RETRIEVE: get file
				printf("Receving ...\n");
				recvFile(sock_control,sock_data, arg);
				printf("xong r ma\n");
			}
			printf("dong data connection\n");
			// Close data connection
			close(sock_data);
		} 
	}
}


int main(int argc, char const *argv[])
{
	int ListenSock, CtrlSock, pid;

	if ((ListenSock = socket_create()) < 0 ) {
		perror("Error creating socket");
		exit(1);
	}		
	
	while(1) {	// wait for client request

		// create new socket for control connection
		if ((CtrlSock = socket_accept(ListenSock))	< 0 )
			break;			
		
		// create child process to do actual file transfer
		if ((pid = fork()) < 0) { 
			perror("Error forking child process");
		} else if (pid == 0) { 
			close(ListenSock);
			ftserve_process(CtrlSock);
			close(CtrlSock);
			exit(0);
		}
		close(CtrlSock);
	}

	close(ListenSock);	
	return 0;
}