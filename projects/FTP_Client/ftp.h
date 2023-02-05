 
#ifndef TIANJING_FTP_H
#define TIANJING_FTP_H
 
#include <stdio.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#define MAX_BUFF 512
#define MAX_CMD_LEN 512
#define MAX_INPUT_SIZE 128

 //Link server
int ftp_connectServer( const char *host, char* re_buf, int port );
int ftp_sendcmd( int sock, char *cmd, void *re_buf, ssize_t *len);
void ftp_readInput(char* user_input, int size);


int ftp_login(int c_sock, const char* host);
 //Disconnect the server
int ftp_quit( int c_sock);
 //Set the representation type
int ftp_type( int c_sock, char mode );
 //Change working directory
int ftp_cwd( int c_sock, char *path );
 // print working directory
int ftp_pwd(int c_sock, char* re_data);
 //List
int ftp_list( int c_sock, char *path, void **data, ssize_t *data_len);
 //download file 
int ftp_retrfile( int c_sock, char *s, char *d ,ssize_t *stor_size);
 //upload files 
int ftp_storfile( int c_sock, char *d, char *s ,ssize_t *stor_size);
 
#endif //TIANJING_FTP_H
 
 
/** Reply Codes by Function Groups
    110 Restart marker reply.
        In this case, the text is exact and not left to the
        particular implementation; it must read:
        MARK yyyy = mmmm
        Where yyyy is User-process data stream marker, and mmmm
        server's equivalent marker (note the spaces between markers
        and "=").
    120 Service ready in nnn minutes.
    125 Data connection already open; transfer starting.
    150 File status okay; about to open data connection.
    200 Command okay.
    202 Command not implemented, superfluous at this site.
    211 System status, or system help reply.
    212 Directory status.
    213 File status.
    214 Help message.
        On how to use the server or the meaning of a particular
        non-standard command.  This reply is useful only to the
        human user.
    215 NAME system type.
        Where NAME is an official system name from the list in the
        Assigned Numbers document.
    220 Service ready for new user.
    221 Service closing control connection.
        Logged out if appropriate.
    225 Data connection open; no transfer in progress.
    226 Closing data connection.
        Requested file action successful (for example, file transfer or file abort).
    227 Entering Passive Mode (h1,h2,h3,h4,p1,p2).
    230 User logged in, proceed.
    250 Requested file action okay, completed.
    257 "PATHNAME" created.

    331 User name okay, need password.
    332 Need account for login.
    350 Requested file action pending further information.

    421 Service not available, closing control connection.
        This may be a reply to any command if the service knows it must shut down.
    425 Can't open data connection.
    426 Connection closed; transfer aborted.
    450 Requested file action not taken.
        File unavailable (e.g., file busy).
    451 Requested action aborted: local error in processing.
    452 Requested action not taken.
        Insufficient storage space in system.
    500 Syntax error, command unrecognized.
        This may include errors such as command line too long.
    501 Syntax error in parameters or arguments.
    502 Command not implemented.
    503 Bad sequence of commands.
    504 Command not implemented for that parameter.
    530 Not logged in.
    532 Need account for storing files.
    550 Requested action not taken.
        File unavailable (e.g., file not found, no access).
    551 Requested action aborted: page type unknown.
    552 Requested file action aborted.
        Exceeded storage allocation (for current directory or dataset).
    553 Requested action not taken.
        File name not allowed.
**/
/**The following are the FTP commands:

    USER <SP> <username> <CRLF>
    PASS <SP> <password> <CRLF>
    ACCT <SP> <account-information> <CRLF>
    CWD  <SP> <pathname> <CRLF>
    CDUP <CRLF>
    SMNT <SP> <pathname> <CRLF>
    QUIT <CRLF>
    REIN <CRLF>
    PORT <SP> <host-port> <CRLF>
    PASV <CRLF>
    TYPE <SP> <type-code> <CRLF>
    STRU <SP> <structure-code> <CRLF>
    MODE <SP> <mode-code> <CRLF>
    RETR <SP> <pathname> <CRLF>
    STOR <SP> <pathname> <CRLF>
    STOU <CRLF>
    APPE <SP> <pathname> <CRLF>
    ALLO <SP> <decimal-integer>
        [<SP> R <SP> <decimal-integer>] <CRLF>
    REST <SP> <marker> <CRLF>
    RNFR <SP> <pathname> <CRLF>
    RNTO <SP> <pathname> <CRLF>
    ABOR <CRLF>
    DELE <SP> <pathname> <CRLF>
    RMD  <SP> <pathname> <CRLF>
    MKD  <SP> <pathname> <CRLF>
    PWD  <CRLF>
    LIST [<SP> <pathname>] <CRLF>
    NLST [<SP> <pathname>] <CRLF>
    SITE <SP> <string> <CRLF>
    SYST <CRLF>
    STAT [<SP> <pathname>] <CRLF>
    HELP [<SP> <string>] <CRLF>
    NOOP <CRLF>

**/