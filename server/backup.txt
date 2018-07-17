#include "client_commons.h"

int c1Port;
int d1Port;
int d2Port;
int sockfd, d1fd, d2fd;
char* stringServerAddress;
struct sockaddr_in servaddr, servd1addr, servd2addr;
char lastMessageSent[50] = "";
int stdineof=0;


int main(int argc, char **argv)
{

	int fd;

	if (argc != 5)
		{
		printf("usage: client <IPaddress> <c1Port><d1Port><d2Port>\n");
		exit(-1);
		}
	stringServerAddress = argv[1];
	c1Port = atoi(argv[2]);
	d1Port = atoi(argv[3]);
	d2Port = atoi(argv[4]);
	

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	d1fd = socket(AF_INET, SOCK_STREAM, 0);
	d2fd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	bzero(&servd1addr, sizeof(servd1addr));
	bzero(&servd2addr, sizeof(servd2addr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(c1Port);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

	servd1addr.sin_family = AF_INET;
	servd1addr.sin_port = htons(d1Port);
	inet_pton(AF_INET, argv[1], &servd1addr.sin_addr);

	servd2addr.sin_family = AF_INET;
	servd2addr.sin_port = htons(d2Port);
	inet_pton(AF_INET, argv[1], &servd2addr.sin_addr);

	//listenD2();
	
	fd=STDIN_FILENO;
	str_cli(fd, sockfd);
	exit(0);
}

void listenD2()
{

	pid_t childpid;
	socklen_t	clilen;
	struct sockaddr_in cliaddr,servaddr;
	int d2active;
	listen(d2fd,10);

	// END initializing sockets

	for(;;)
	{
		clilen=sizeof(cliaddr);
		if((d2active=accept(d2fd,(struct sockaddr *) &cliaddr,&clilen))<0)
		{
			printf("erreur accept\n");
			exit(-1);
		}
		if((childpid=fork())==0)
		{	
			writeToFd("receivinf on d2\n", STDOUT_FILENO);
			close(d2fd);
			//handleClient(c1);
			exit(0);
		}
		close(d2active);
	}
}



void str_cli(int fd, int sockfd)
{
	int		maxfdp1; // stdineof passe Ã  1 si on a exÃ©cutÃ© shutdown sur la socket
	fd_set		rset;
	char		sendline[MAXLINE], recvline[MAXLINE];
	int 		n;

	//if (fd==STDIN_FILENO) printf("\nentrez votre chaine :\n");

	
	FD_ZERO(&rset);
	writeToFd("BONJ\n", sockfd);
	
	for ( ; ; )
		{
		// Si on n'a pas appelÃ© shutdown sur la socket
		// C'est-Ã -dire qu'on a encore des donnÃ©es Ã  lire sur l'entrÃ©e de donnÃ©es
		if (stdineof==0)
			{
			
			FD_SET(fd, &rset);  // On ajoute l'entrÃ©e de donnÃ©es Ã  rset
			maxfdp1 = max(fd, sockfd) + 1; // on calcule la valeur du plus grand descripteur Ã  laquelle on ajoute 1.
			}
		else maxfdp1=sockfd+1; // Sinon, il n'y a plus rien Ã  lire sur l'entrÃ©e de donnÃ©es, seule la socket sera surveillÃ©e

		FD_SET(sockfd, &rset); // On ajoute le descripteur de socket Ã  rset

		// On surveille le descripteur de la socket et Ã©ventuellement celui de l'entrÃ©e de donnÃ©es
		select(maxfdp1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(sockfd, &rset))
			{	/* socket is readable */
			n=read(sockfd, recvline, MAXLINE);
			if (n==0) // Cas oÃ¹ read retourne une valeur nulle
				{
				// Si on a appelÃ© shutdown sur la socket (stdineof==1)
				// alors tout est normal, c'est que le serveur a terminÃ© son envoi de donnÃ©es
				if (stdineof==1)
					return; // On sort de la fonction
				else // Sinon, si on n'a pas appelÃ© shutdown
					{ // C'est que le serveur est tombÃ©
					printf("Le serveur a quitte prematurement\n");
					exit(-1);
					}
				}
			else if (n<0)
				{
				printf("Erreur de socket\n");
				exit(-1);
				}
			recvline[n]='\0';

			response(recvline, STDOUT_FILENO);
			}

		if (FD_ISSET(fd, &rset)) // EvÃ¨nement sur l'entrÃ©e de donnÃ©es
			{
			// Si la lecture sur l'entrÃ©e de donnÃ©es renvoie 0
			// (fin de fichier ou fin de saisie)
			if ((n=read(fd, sendline,MAXLINE)) == 0)
				{
				// On ferme la socket Ã  moitiÃ©, seulement en Ã©criture
				// Pour continuer Ã  recevoir les rÃ©ponses du serveur
				// Tout en signalant au serveur que le client a terminÃ©
				shutdown(sockfd,SHUT_WR);
				stdineof=1; // On marque le fait que shutdown a Ã©tÃ© appelÃ©
				// Il n'y a plus rien Ã  lire sur l'entrÃ©e de donnÃ©es
				// On ne surveille plus son descripteur
				// On le retire de rset.
				FD_CLR(fd,&rset);
				continue;
				}
			//printf("debug oooooooooooooooooooooooooooook, fd = %d\n", sockfd);
			write(sockfd, sendline, n);
			sendline[n] = '\0';
			strcpy(lastMessageSent, sendline);
			}
		}
}



void response(char* cmd, int out_fd)
{
   	
	switch(lookup_cmd(cmd))
	{
		case RDY: upload(lastMessageSent);break;
		case LS: ls(out_fd);break;
		case PWD: pwd(out_fd);break;
		case FBDN: writeToFd("Impossible to create a file\n", out_fd); break;
		default : writeToFd(cmd, out_fd); break;
	}
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

void ls(int out_fd)
{	
	executeLinuxCommand("/bin/ls", "ls");
}

void pwd(int out_fd)
{
	
	executeLinuxCommand("/bin/pwd", "");
}

void upload(char* fileName)
{
	connect(d1fd, (struct sockaddr*) &servd1addr, sizeof(servd1addr));
	removeNewLine(fileName);
	sendFile(d1fd, fileName);

	shutdown(d1fd,SHUT_WR);
	//stdineof = 1;

	//int fd=STDIN_FILENO;
	bzero(&servaddr, sizeof(servaddr));
	bzero(&servd1addr, sizeof(servd1addr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(c1Port);
	inet_pton(AF_INET, stringServerAddress, &servaddr.sin_addr);

	connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	//str_cli(fd, sockfd);
}

void sendFile(int out_fd, char* fileName)
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

void removeNewLine(char* text)
{
	text[strlen(text) - 1] = '\0';
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
















