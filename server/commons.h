#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#define BUF_SIZE 8192

#define CLIENT_SEND_BONJ "BONJ"
#define SERVER_SENT_LOGIN "LOGIN"
#define SERVER_SENT_PASSWD "PASSWD"

#define CLIENT_SENT_LS "rls"


#define RLS_FILE "rls_file.txt"
#define RPWD_FILE "rpwd_file.txt"

#define	max(a,b)	((a) > (b) ? (a) : (b))

/* Infos enumeration */
typedef enum infoslist 
{ 
  RDY, FBDN, LS, PWD
} infoslist;

/* String mappings for infoslist */
static const char *infoslist_str[] = 
{
  "RDY\n", "FBDN\n", "ls\n", "pwd\n"
};

/* Commands enumeration */
typedef enum cmdlist 
{ 
  RLS, RPWD, UPLD, LS_CMD, PWD_CMD, DOWNL
} cmdlist;

/* String mappings for cmdlist */
static const char *cmdlist_str[] = 
{
  "rls","rpwd", "upld", "ls", "pwd", "downl"
};

char* byeMessage = "\n\nToo many attempt, Bye\n\n";

int lookup_cmd(char *cmd);
void response(char* cmd, int out_fd);
void writeToFd(char* message, int out_fd);
int lookup(char *needle, const char **haystack, int count);
void  handleClient(int sockfd);
int logUser(char* login, char* password);
char* getMessageToSend(char* clientMessage, int* needToCLose);
void handleCommand(int sockfd, char* clientMessage);
void removeNewLine(char* text);
void sendfile(int out_fd, char* fileName);
void rls(int sockfd);
int executeLinuxCommand(char* path, char* arg);
void rpwd(int out_fd);
void upload(char* fileName, int out_fd);
char* getInfoFromSymbol(int symbol);
void listeningD1();
void listeningC1();
void ls(int out_fd);
void pwd(int out_fd);
void download(char* fileName, int out_fd);

