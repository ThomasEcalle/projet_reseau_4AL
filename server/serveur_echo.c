
#include "commons.h"

int totaldonnees=0;
char lastMessageSent[50] = CLIENT_SEND_BONJ;
int lastCommand = 400;
char login[50];
int isUserLoggedIn = 0;
int loginTries = 2;
int lfd,c1;
int d1, d1active, d2, d2active;

pid_t childpid;
socklen_t	clilen;
struct sockaddr_in cliaddr,servaddr;
struct sockaddr_in d1addr, d2addr; 
struct sigaction act;


int main(int argc,char **argv)
{


	if (argc!=4)
	{
		printf("Usage : <c1 Port><d1 Port><d2 Port>\n");
		exit(-1);
	}

	act.sa_handler=SIG_DFL;
	act.sa_flags=SA_NOCLDWAIT;
	sigaction(SIGCHLD,&act,NULL);

	// Initializing sockets
	lfd=socket(AF_INET,SOCK_STREAM,0);
	d1=socket(AF_INET,SOCK_STREAM,0);
	d2=socket(AF_INET,SOCK_STREAM,0);

	bzero(&servaddr,sizeof(servaddr));
	bzero(&d1addr,sizeof(d1addr));
	bzero(&d2addr,sizeof(d2addr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(atoi(argv[1]));

	d1addr.sin_family = AF_INET;
	d1addr.sin_addr.s_addr=htonl(INADDR_ANY);
	d1addr.sin_port=htons(atoi(argv[2]));

	d2addr.sin_family = AF_INET;
	d2addr.sin_addr.s_addr=htonl(INADDR_ANY);
	d2addr.sin_port=htons(atoi(argv[3]));

	bind(lfd,(struct sockaddr *) &servaddr, sizeof(servaddr));
	bind(d1,(struct sockaddr *) &d1addr, sizeof(d1addr));
	bind(d2,(struct sockaddr *) &d2addr, sizeof(d2addr));
	
	
	perror("bind");
	
	char listeningMessage[100] = "";
	sprintf(listeningMessage,"Listening on :\nc1 : %s\nd1 : %s\nd2 : %s\n\n", argv[1],argv[2],argv[3]);
	writeToFd(listeningMessage, STDOUT_FILENO);

	listen(lfd,10);

	// END initializing sockets

	for(;;)
	{
		clilen=sizeof(cliaddr);
		if((c1=accept(lfd,(struct sockaddr *) &cliaddr,&clilen))<0)
		{
			printf("erreur accept\n");
			exit(-1);
		}
		if((childpid=fork())==0)
		{	
					
			close(lfd);
			handleClient(c1);
			exit(0);
		}
		close(c1);
	}

}


void response(char* cmd, int out_fd)
{
	//writeToFd("On respooonse\n", STDOUT_FILENO);
	switch(lookup_cmd(cmd))
	{
		case RLS: rls(out_fd); break;
		case RPWD: rpwd(out_fd); break;
		case LS_CMD: ls(out_fd); break;
		case PWD_CMD: pwd(out_fd);break;
		case UPLD: 
			lastCommand = UPLD;
			writeToFd("\nPlease give us file name:\n", out_fd);
		break;
		case DOWNL: 
			lastCommand = DOWNL;
			writeToFd("\nPlease give us file name:\n", out_fd);
		break;
		default: 
			switch(lastCommand)
			{
				case UPLD: upload(cmd, out_fd); break;
				case DOWNL: download(cmd, out_fd); break;
				default: writeToFd("\nUnknown command\n", out_fd); break;
			}
			
		break;
	}
}

void  handleClient(int sockfd)
{
	ssize_t n;
	char buf[100],final[150];

	while((n=read(sockfd,buf,100))>0)
	{
		
		totaldonnees+=n;
		buf[n] = '\0';
		int needToClose = 0;
		
		if (isUserLoggedIn == 0)
		{
			char* message = getMessageToSend(buf, &needToClose);

			if (needToClose) 
			{			
				writeToFd(byeMessage, sockfd);
				close(sockfd);
			}
			else writeToFd(message, sockfd);
		}
		else
		{
			removeNewLine(buf);
			response(buf,sockfd);
		}
	}
	printf("User out\n");

}

int lookup_cmd(char *cmd)
{

  const int cmdlist_count = sizeof(cmdlist_str)/sizeof(char *);
  return lookup(cmd, cmdlist_str, cmdlist_count);
}

void writeToFd(char* message, int out_fd)
{
	write(out_fd,message,strlen(message));
}

int lookup(char *needle, const char **haystack, int count)
{
  int i;
  for(i=0;i<count; i++){
    if(strcmp(needle,haystack[i])==0)return i;
  }
  return -1;
}

char* getInfoFromSymbol(int symbol)
{
	return infoslist_str[symbol];
}

void ls(int out_fd)
{
	writeToFd(getInfoFromSymbol(LS), out_fd);
}

void pwd(int out_fd)
{
	writeToFd(getInfoFromSymbol(PWD), out_fd);
}

void download(char* fileName, int out_fd)
{
	connect(d2, (struct sockaddr*) &d2addr, sizeof(d2addr));	
	writeToFd("totoro\n", STDOUT_FILENO);
	writeToFd("download test\n", d2);
}


void upload(char* fileName, int out_fd)
{
	FILE* file = NULL;

	file = fopen(fileName, "w");
	if (file != NULL)
	{
		writeToFd(getInfoFromSymbol(RDY), out_fd);
		struct sockaddr_in cliaddr;
		socklen_t clilen;
		pid_t childpid;
		
		ssize_t n;
		char buf[100];
		listen(d1,10);


		for(;;)
		{
			clilen=sizeof(cliaddr);
			if((d1active=accept(d1,(struct sockaddr *) &cliaddr,&clilen))<0)
			{
				printf("erreur accept\n");
				exit(-1);
			}
			if((childpid=fork())==0)
			{
				close(d1);
				while((n=read(d1active,buf,100))>0)
				{
		
					fputs(buf, file);
				}
				printf("User out of the d1 socket\n");
				exit(0);
			}
			close(d1active);
		}

		fclose(file);
	}
	else
	{
		writeToFd(getInfoFromSymbol(FBDN), out_fd);
	}
	
}

void rpwd(int out_fd)
{
	FILE* file = NULL;

	file = fopen(RPWD_FILE, "w");

	if (file != NULL)
	{
		dup2(fileno(file),STDOUT_FILENO);	
		int result = executeLinuxCommand("/bin/pwd", "");

		fclose(file);
		if (result)
		{
			sendfile(out_fd, RPWD_FILE);
		}
			
	}
	else
	{
		printf("File opening impossible\n");
	}
}

void rls(int sockfd)
{
	FILE* file = NULL;

	file = fopen(RLS_FILE, "w");

	if (file != NULL)
	{
		dup2(fileno(file),STDOUT_FILENO);	
		int result = executeLinuxCommand("/bin/ls", "ls");
		
		//char* message1 = "Reading text with size :\n";
		//char concat[100];
		//sprintf(concat,"size %d",size);
		fclose(file);
		if (result)
		{
			sendfile(sockfd, RLS_FILE);
		}
			
	}
	else
	{
		printf("File opening impossible\n");
	}
}

int executeLinuxCommand(char* path, char* arg)
{
	pid_t pid;

	pid = fork();
	if (pid < 0)
	{
		printf("Fork error");
		exit(1);
	}
	else if (pid == 0) /* We are in the child. */
	{
		//In the child, about to call ps using execlp.
		execlp(path,arg,(char *) 0);
		/*  If execlp() is successful, we should not reach this next line. */
		
		return 0;
	}
	else  /* We are in the parent. */
	{
		wait(0);               /* Wait for the child to terminate. */
		return 1;
	}

}



char* getMessageToSend(char* clientMessage, int* needToCLose) 
{
	
	if (strcmp(lastMessageSent, CLIENT_SEND_BONJ) == 0)
	{
		strcpy(lastMessageSent, SERVER_SENT_LOGIN);
		return "Please enter your login\n";
	}
	else if (strcmp(lastMessageSent, SERVER_SENT_LOGIN) == 0)
	{
		strcpy(lastMessageSent, SERVER_SENT_PASSWD);
		removeNewLine(clientMessage);
		strcpy(login, clientMessage);
		return "Please enter your password\n";
	}
	else
	{
		removeNewLine(clientMessage);
		int isLogged = logUser(login, clientMessage);
		if (isLogged)
		{
			isUserLoggedIn = 1;
			return "logged in\n\n\n";
		}
		if (loginTries == 0)
		{
			*needToCLose = 1;
			return NULL;
		}

		loginTries -= 1;
		strcpy(lastMessageSent, SERVER_SENT_LOGIN);

		return "\nBad credentials, Please enter your login again :\n";
	}
	
}

int logUser(char* login, char* password)
{
	FILE* file = NULL;
	file = fopen("credentials.txt", "r");
	int found = 0;

	if (file != NULL)
	{
		char pseudo[100];
		char pass[100];

		while(fscanf(file, "%s %s", pseudo, pass) == 2)
		{
			if (strcmp(pseudo, login) == 0 && strcmp(pass, password) == 0)
			{
				found = 1;
				break;
			}
		}
		

		fclose(file);
	}
	else
	{
		printf("File opening impossible\n");
		return found;
	}
	return found;
}

void removeNewLine(char* text)
{
	text[strlen(text) - 1] = '\0';
}

void sendfile(int out_fd, char* fileName)
{
	FILE* file = NULL;
	file = fopen(fileName, "r");

	if (file != NULL)
	{
		char text[BUF_SIZE] = "";

		while(fgets(text, BUF_SIZE, file) != NULL)
		{
			writeToFd(text, out_fd);
		}
		

		fclose(file);
	}
	else
	{
		printf("File opening impossible\n");
	}
}

























