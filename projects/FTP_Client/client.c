#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ftp.h"
 
#define PORT 21

//gcc -o ftp ftp.h ftp.c client.c


int main(int argc, char const *argv[]){
	
	if (argc != 2) {
		printf("usage: ./ftp host(FTP server ip)\n");
		exit(0);
	}
    // connect to server
    char  cmd[MAX_BUFF],response[MAX_BUFF], input[MAX_CMD_LEN];
	int     c_sock;

    c_sock = ftp_connectServer( argv[1], response, PORT );
    if ( c_sock == -1 ) {
        printf("ftp: Can't connect to `%s:ftp'\n",argv[1]);
        return -1;
    }
	printf("Connected to %s\n", argv[1]);
    printf("%s",response);

	// login
	if( ftp_login(c_sock, argv[1]) == -1) {
		printf("ftp: Login failed\n");
	}

	while (1) { // loop until user types quit
		printf("fpt> ");
		fflush(stdout);

		ftp_readInput(input, MAX_INPUT_SIZE);

		if(strncmp(input, "quit", 4) == 0) {
			ftp_quit(c_sock);
			break;
		} else if(strncmp(input, "help", 4) == 0) { // help -> list command
			printf("Commands may be abbreviated.  Commands are:\n");
			printf("%-5s: %s\n", "ls", "get list file in directory on remote side");
			printf("%-5s: %s\n", "pwd", "print working directory");
			printf("%-5s: %s\n", "cd", "change directory");
			printf("%-5s: %s\n", "get", "download file");
			printf("%-5s: %s\n", "put", "upload file");
			printf("%-5s: %s\n", "quit", "logout");

		} else if(strncmp(input, "ls", 2) == 0) { // Get list file
			void *data;
			ssize_t len;
			if(input[2] == ' ') {
				ftp_list(c_sock, input+3, &data, &len);
			} else {
				ftp_pwd(c_sock, response);
				ftp_list(c_sock, response, &data, &len);
			}
			char *list = (char*)malloc(len);
			memcpy(list, data, len);
			printf("%s",list);
		} else if(strncmp(input, "pwd", 3) == 0) { // print working directory
			ftp_pwd(c_sock, response);
			printf("Remote directoy: %s\n",response);
		} else if(strncmp(input, "cd ", 3) == 0) { // change directory
			ftp_cwd(c_sock, input+3);
		} else if(strncmp(input, "get ", 4) == 0) { // Download files: get REMOTE_FILE_PATH LOCAL_FILE_PATH
			ssize_t len;
			char local_file_path[MAX_BUFF] = {0}, 
			     remote_file_path[MAX_BUFF] = {0};
			
			sscanf(input + 4,"%s %s", remote_file_path, local_file_path);
			if(local_file_path[0] == '\0') {
				strcpy(local_file_path, remote_file_path);
			}
			printf("local: %s\tremote: %s\n", local_file_path, remote_file_path);
			ftp_retrfile(c_sock, remote_file_path, local_file_path ,&len);

		}  else if(strncmp(input, "put ", 4) == 0) { // upload files: put LOCAL_FILE_PATH REMOTE_FILE_PATH
			ssize_t len;
			char local_file_path[MAX_BUFF] = {0}, 
			     remote_file_path[MAX_BUFF] = {0};
			
			sscanf(input + 4,"%s %s", local_file_path, remote_file_path);
			if(remote_file_path[0] == '\0') {
				strcpy(remote_file_path, local_file_path);
			}
			printf("local: %s\tremote: %s\n", local_file_path, remote_file_path);
			ftp_storfile(c_sock, local_file_path, remote_file_path ,&len);
		}  else {
			printf("?Invalid command.\n");
		}

	} // loop back to get more user input
	return 0;
 
}