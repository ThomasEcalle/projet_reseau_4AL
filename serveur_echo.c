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

int totaldonnees=0;

void str_echo(int);

main(int argc,char **argv)
{
int lfd,cfd;
pid_t childpid;
socklen_t	clilen;
struct sockaddr_in cliaddr,servaddr;
struct sigaction act;

if (argc!=2)
	{
	printf("Usage : serveur Port\n");
	exit(-1);
	}

act.sa_handler=SIG_DFL;
act.sa_flags=SA_NOCLDWAIT;
sigaction(SIGCHLD,&act,NULL);

lfd=socket(AF_INET,SOCK_STREAM,0);

bzero(&servaddr,sizeof(servaddr));

servaddr.sin_family = AF_INET;
servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
servaddr.sin_port=htons(atoi(argv[1]));

bind(lfd,(struct sockaddr *) &servaddr, sizeof(servaddr));	
perror("bind");

listen(lfd,10);

for(;;)
	{
	clilen=sizeof(cliaddr);
	if((cfd=accept(lfd,(struct sockaddr *) &cliaddr,&clilen))<0)
		{
		printf("erreur accept\n");
		exit(-1);
		}
	if((childpid=fork())==0)
		{
		close(lfd);
		str_echo(cfd);
		exit(0);
		}
	close(cfd);
}

}

void  str_echo(int sockfd)
{
	ssize_t n;
	char buf[100],final[150];

	while((n=read(sockfd,buf,100))>0)
	{
		totaldonnees+=n;
		buf[n]='\0';
		//printf("envoi dans socket de %s\n",buf);
		write(sockfd,buf,n);
	}
	printf("total donnÃ©es reÃ§ues: %d\n",totaldonnees);

}