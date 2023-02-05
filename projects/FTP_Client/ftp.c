#include "ftp.h"
void replace_multi_space_with_single_space(char *str)
{
    char *dest = str;  /* Destination to copy to */

    /* While we're not at the end of the string, loop... */
    while (*str != '\0')
    {
        /* Loop while the current character is a space, AND the next
         * character is a space
         */
        while (*str == ' ' && *(str + 1) == ' ')
            str++;  /* Just skip to next character */

       /* Copy from the "source" string to the "destination" string,
        * while advancing to the next character in both
        */
       *dest++ = *str++;
    }

    /* Make sure the string is properly terminated */
    if(*(dest-1) == ' ' ){
        *(dest - 1) = '\0';
    } else {
        *dest = '\0';
    }
}

/** 
 * Read input from command line
 */
void ftp_readInput(char* user_input, int size)
{
	memset(user_input, 0, size);
	int n = read(STDIN_FILENO,user_input,size);
	user_input[n] = '\0';		
    
	/* Remove trailing return and newline characters */
	if(user_input[n - 1] == '\n') {
		user_input[n - 1] = '\0';
    }
	if(user_input[n - 2] == '\r') {
		user_input[n - 2] = '\0';
    }

	replace_multi_space_with_single_space(user_input);
} 

 //Create a socket and return
int socket_connect(const char *host,int port)
{
    struct sockaddr_in address;
    int s, opvalue;
    socklen_t slen;
 
    opvalue = 8;
    slen = sizeof(opvalue);
    memset(&address, 0, sizeof(address));
 
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ||
        setsockopt(s, IPPROTO_IP, IP_TOS, &opvalue, slen) < 0) // IP_TOS -> ipv4
        // int setsockopt(int socket, int level, int option_name,const void *option_value, socklen_t option_len);
        return -1;
 
         //Set receiving and sending timeout
    struct timeval timeo = {15, 0};
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo));
 
    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short)port);
 
    struct hostent* server = gethostbyname(host);
    if (!server)
        return -1;

    memcpy(&address.sin_addr.s_addr, server->h_addr, server->h_length);
 
    if (connect(s, (struct sockaddr*) &address, sizeof(address)) == -1)
        return -1;
 
    return s;
}
 //Connect to a ftp server and return socket
int ftp_connectServer( const char *host, char* re_buf,int port )
{
    int       ctrl_sock;
    char      buf[512];
    int       result;
    ssize_t   len;
 
    ctrl_sock = socket_connect(host, port);
    if (ctrl_sock == -1) {
        return -1;
    }
 
    len = recv( ctrl_sock, buf, 512, 0 );
    buf[len] = 0;
    sscanf( buf, "%d", &result );
    if ( result != 220 ) {
        close( ctrl_sock );
        return -1;
    }
    sprintf(re_buf, "%s", buf);
    return ctrl_sock;
}
 //Send command and return result
int ftp_sendcmd( int sock, char *cmd, void *re_buf, ssize_t *len)
{
    char        buf[512];
    ssize_t     r_len;
    int re_code;
 
    if ( send( sock, cmd, strlen(cmd), 0 ) == -1 )
        return -1;
 
    r_len = recv( sock, buf, 512, 0 );
    if ( r_len < 1 ) return -1;
    buf[r_len] = 0;
 
    if (len != NULL) *len = r_len;
    if (re_buf != NULL) sprintf(re_buf, "%s", buf);
    sscanf( buf, "%d", &re_code );

    return re_code;
}

 //Log in to the ftp server
int ftp_login(int c_sock, const char* host)
{
    char user[50], re_buf[MAX_BUFF],cmd[MAX_CMD_LEN];
    int re_code;
    ssize_t len;
    printf("Name (%s): ", host);
	fflush(stdout);
	ftp_readInput(user, MAX_BUFF);
	sprintf(cmd, "USER %s\r\n", user);
	re_code = ftp_sendcmd(c_sock, cmd, re_buf, &len);
	re_buf[len] = '\0';

	if( re_code == 230) {
		printf("%s", re_buf);
	} else if (re_code == 331) {
		printf("%s",re_buf);
		fflush(stdout);
		char *pwd = getpass("Password: ");	
		sprintf( cmd, "PASS %s\r\n", pwd );
		re_code = ftp_sendcmd( c_sock, cmd, re_buf, &len );
        printf("%s",re_buf);
        if(re_code == 530) { // login failed
            return -1;
        }
	}
    return 0;
}
int create_datasock( int ctrl_sock )
{
    int     lsn_sock;
    int     port;
    int     len;
    struct sockaddr_in sin;
    char    cmd[MAX_CMD_LEN], re_buf[MAX_BUFF];
    ssize_t re_len;
 
    lsn_sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( lsn_sock == -1 ) return -1;
    memset( (char *)&sin, 0, sizeof(sin) );
    sin.sin_family = AF_INET;
    if( bind(lsn_sock, (struct sockaddr *)&sin, sizeof(sin)) == -1 ) {
        close( lsn_sock );
        return -1;
    }
 
    if( listen(lsn_sock, 2) == -1 ) {
        close( lsn_sock );
        return -1;
    }
 
    len = sizeof( struct sockaddr );
    if ( getsockname( lsn_sock, (struct sockaddr *)&sin, (socklen_t *)&len ) == -1 )
    {
        close( lsn_sock );
        return -1;
    }
    port = sin.sin_port;
 
    if( getsockname( ctrl_sock, (struct sockaddr *)&sin, (socklen_t *)&len ) == -1 )
    {
        close( lsn_sock );
        return -1;
    }
 
    sprintf( cmd, "PORT %d,%d,%d,%d,%d,%d\r\n", // PORT h1,h2,h3,h4,p1,p2 -> h1->h4 chia địa chỉ thành 8bit và gửi ở dạng %d
             sin.sin_addr.s_addr&0x000000FF,
             (sin.sin_addr.s_addr&0x0000FF00)>>8,
             (sin.sin_addr.s_addr&0x00FF0000)>>16,
             (sin.sin_addr.s_addr&0xFF000000)>>24,
             port>>8, port&0xff );
 
    if ( ftp_sendcmd( ctrl_sock, cmd, re_buf, &re_len) != 200 ) {
        close( lsn_sock );
        return -1;
    }
    return lsn_sock;
}
 //Connect to PASV interface
int ftp_pasv_connect( int c_sock )
{
    int     r_sock, addr[6], send_re;
    ssize_t len;
    char    buf[MAX_CMD_LEN], re_buf[MAX_BUFF];
 
    //Set PASV passive mode
    memset(buf, 0, sizeof(buf));
    sprintf( buf, "PASV\r\n");
    send_re = ftp_sendcmd( c_sock, buf, re_buf, &len);
    printf("%s",re_buf);
    if (send_re != -1) {
        sscanf(re_buf, "%*[^(](%d,%d,%d,%d,%d,%d)",&addr[0],&addr[1],&addr[2],&addr[3],&addr[4],&addr[5]);
    }
 
    //Connect to PASV port
    memset(buf, 0, sizeof(buf));
    sprintf( buf, "%d.%d.%d.%d",addr[0],addr[1],addr[2],addr[3]);
    r_sock = socket_connect(buf,addr[4]*256+addr[5]);
 
    return r_sock;
}
 //Indicates the type
int ftp_type( int c_sock, char mode )
{
    char    cmd[MAX_CMD_LEN], re_buf[MAX_BUFF];
    ssize_t len;
    sprintf( cmd, "TYPE %c\r\n", mode );
    int re_code = ftp_sendcmd( c_sock, cmd, re_buf, &len);
    printf("%s",re_buf);
    return re_code;
}
 //Change working directory
int ftp_cwd( int c_sock, char *path )
{
    char    cmd[MAX_CMD_LEN], re_buf[MAX_BUFF];
    int     re_code;
    ssize_t len;
    sprintf( cmd, "CWD %s\r\n", path );
    re_code = ftp_sendcmd( c_sock, cmd, re_buf, &len);
 
    re_buf[len] = '\0';
    printf("%s", re_buf);
    return 0;
}
int ftp_pwd(int c_sock, char* re_data)
{
    char    cmd[MAX_CMD_LEN];
    int     re;
    sprintf( cmd, "PWD \r\n");
    char re_buf[MAX_BUFF];
    ssize_t len;

    re = ftp_sendcmd( c_sock, cmd , re_buf, &len);
    re_buf[len] = '\0';

    if(re != 257) {
        return re;
    }
    char*wd = strchr(re_buf,'\"');
    char*end_wd = strchr(wd+1, '\"');
    *end_wd ='\0';
	sprintf(re_data,"%s", wd+1);
    return re;
}

 //List
int ftp_list( int c_sock, char *path, void **data, ssize_t *data_len)
{
    int     d_sock;
    char    buf[MAX_BUFF], re_buf[MAX_BUFF];
    int     re_code;
    int     result;
    ssize_t     len,buf_len,total_len;
 
    //Connect to PASV interface
    d_sock = ftp_pasv_connect(c_sock);
    if (d_sock == -1) {
        return -1;
    }
    
    //Send LIST command
    bzero(buf, sizeof(buf));
    sprintf( buf, "LIST %s\r\n", path);
    re_code = ftp_sendcmd( c_sock, buf, re_buf, &len );
    if (re_code >= 300 || re_code == 0){

        return re_code;
    }
 
    len=total_len = 0;
    buf_len = MAX_BUFF;
    void *re_data = malloc(buf_len);
    while ( (len = recv( d_sock, buf, 512, 0 )) > 0 )
    {
        if (total_len+len > buf_len)
        {
            buf_len *= 2;
            void *re_buf_n = malloc(buf_len);
            memcpy(re_buf_n, re_data, total_len);
            free(re_data);
            re_data = re_buf_n;
        }
        memcpy(re_data+total_len, buf, len);
        total_len += len;
    }
    close( d_sock );
 
    //Receive the return value from the server
    bzero(buf, sizeof(buf));
    len = recv( c_sock, buf, 512, 0 );
    buf[len] = 0;
    sscanf( buf, "%d", &result );
    if ( result != 226 )
    {
        free(re_data);
        return result;
    }

    *data = re_data;
    *data_len = total_len;
 
    return 0;
}
 //download file 
int ftp_retrfile( int c_sock, char *s, char *d ,ssize_t *retr_size)
{
    int     d_sock;
    ssize_t     len,write_len;
    char    buf[MAX_BUFF], re_buf[MAX_BUFF];
    int     handle;
    int     result;
 
    //Open local file
    handle = open( d,  O_WRONLY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE );
 
    if ( handle == -1 ) {
        printf("ftp: Can't open file %s\n", d);
        return -1;
    }
    //Set the transmission mode
    ftp_type(c_sock, 'I');
 
    //Connect to PASV interface
    d_sock = ftp_pasv_connect(c_sock);
    if (d_sock == -1)
    {
        printf("ftp: Can't passive connect\n");
        close(handle);
        return -1;
    }
 
    //Send RETR command
    memset(buf,0, sizeof(buf));
    sprintf( buf, "RETR %s\r\n", s );
    result = ftp_sendcmd( c_sock, buf, re_buf, &len );
    printf("%s",re_buf);
 
    //Start dowload data to PASV
    memset(buf,0, sizeof(buf));
    clock_t start = clock();
    while ( (len = recv( d_sock, buf, MAX_BUFF, 0 )) > 0 ) {
        write_len = write( handle, buf, len );
        if (write_len != len)
        {
            close( d_sock );
            close( handle );
            return -1;
        }
 
        if (retr_size != NULL)
        {
            *retr_size += write_len;
        }
    }
    clock_t end = clock();
    double download_time = ((double)(end - start))/CLOCKS_PER_SEC;

    close( d_sock );
    close( handle );
    
    //Receive the return value from the server
    memset(buf, 0, sizeof(buf));
    len = recv( c_sock, buf, MAX_BUFF, 0 );
    buf[len] = '\0';
    printf("%s",buf);
    sscanf( buf, "%d", &result );
    printf("%ld bytes received in %lfs (%.4lf KiB/s)\n", *retr_size, download_time, *retr_size/(1024*download_time));

    return result;
}
 //upload files: put LOCAL_FILE_PATH REMOTE_FILE_PATH
int ftp_storfile( int c_sock, char *s, char *d ,ssize_t *stor_size)
{
    int     d_sock, handle, send_re, result;
    char    buf[MAX_BUFF], re_buf[MAX_BUFF]; 
    ssize_t     len,send_len;
    
    //Open local file
    handle = open( s,  O_RDONLY);
    if ( handle == -1 ) {
        printf("ftp: Can't open file %s\n", d);
        return -1;
    }

    //Set the transmission mode
    ftp_type(c_sock, 'I');
 
    //Connect to PASV interface
    d_sock = ftp_pasv_connect(c_sock);
    if (d_sock == -1)
    {
        printf("ftp: Can't passive connect\n");
        close(handle);
        return -1;
    }
 
    //Send STOR command
    memset(buf,0 , sizeof(buf));
    sprintf( buf, "STOR %s\r\n", d );
    send_re = ftp_sendcmd( c_sock, buf, re_buf, &len );
    re_buf[len] = '\0';
    printf("%s",re_buf);
 
    //Start writing data to the PASV channel
    memset(buf,0 , sizeof(buf));
    clock_t start = clock();
    while ( (len = read( handle, buf, MAX_BUFF)) > 0)
    {
        send_len = send(d_sock, buf, len, 0);
        if (send_len != len)
        {
            close( d_sock );
            close( handle );
            return -1;
        }
 
        if (stor_size != NULL)
        {
            *stor_size += send_len;
        }
    }
    clock_t end = clock();
    double upload_time = ((double)(end - start))/CLOCKS_PER_SEC;

    close( d_sock );
    close( handle );
 
    //Receive the return value from the server
    memset(buf,0 , sizeof(buf));
    len = recv( c_sock, buf, MAX_BUFF, 0 );
    buf[len] = '\0';
    printf("%s", buf);
    sscanf( buf, "%d", &result );
    printf("%ld bytes send in %lfs (%.4lf KiB/s)\n", *stor_size, upload_time, *stor_size/(1024*upload_time));
    
    return result;
}

//Disconnect the server
int ftp_quit( int c_sock)
{
    int re_code = 0;
    char re_buf[MAX_BUFF];
    ssize_t len;
    re_code = ftp_sendcmd( c_sock, "QUIT\r\n", re_buf, &len );
    re_buf[len] = '\0';
    printf("%s",re_buf);
    close( c_sock );
    return re_code;
}