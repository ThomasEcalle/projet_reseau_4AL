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
#define	max(a,b)	((a) > (b) ? (a) : (b))
#define MAXLINE	500


/* Commands enumeration */
typedef enum cmdlist 
{ 
  RDY, FBDN, LS, PWD
} cmdlist;

/* String mappings for cmdlist */
static const char *cmdlist_str[] = 
{
  "RDY\n","FBDN\n", "ls\n", "pwd\n"
};


void removeNewLine(char* text);
void str_cli(int, int);
void response(char* cmd, int out_fd);
int lookup_cmd(char *cmd);
void writeToFd(char* message, int out_fd);
int lookup(char *needle, const char **haystack, int count);
void sendFile(int out_fd, char* fileName);
void upload(char* fileName);
int executeLinuxCommand(char* path, char* arg);
void ls(int out_fd);
void pwd(int out_fd);


