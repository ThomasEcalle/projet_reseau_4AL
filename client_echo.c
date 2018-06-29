#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<sys/types.h>
#include	<sys/wait.h>
#include	<signal.h>
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<errno.h>
#include 	<string.h>
#include	<unistd.h>
#include	<fcntl.h>

#define	max(a,b)	((a) > (b) ? (a) : (b))
#define MAXLINE	500

void str_cli(int, int);

int
main(int argc, char **argv)
{
	int	sockfd;
	struct sockaddr_in servaddr;
	int fd;

	if (argc != 4)
		{
		printf("usage: client <IPaddress> <Port> <source_donnÃ©es>\n");
		exit(-1);
		}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

	if (strcmp(argv[3],"stdin")!=0)
		{
		fd=open(argv[3],O_RDONLY);
		if (fd<0)
			{
			printf("Erreur d'ouverture du fichier %s\n",argv[3]);
			exit(-1);
			}
		}
	else fd=STDIN_FILENO;

	str_cli(fd, sockfd);
	exit(0);
}

void str_cli(int fd, int sockfd)
{
int		maxfdp1,stdineof=0; // stdineof passe Ã  1 si on a exÃ©cutÃ© shutdown sur la socket
fd_set		rset;
char		sendline[MAXLINE], recvline[MAXLINE];
int 		n;

if (fd==STDIN_FILENO) printf("\nentrez votre chaine :\n");

FD_ZERO(&rset);
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
				printf("str_cli: serveur termine prematurement\n");
				exit(-1);
				}
			}
		else if (n<0)
			{
			printf("Erreur de socket\n");
			exit(-1);
			}
		recvline[n]='\0';
		write(STDOUT_FILENO,recvline,strlen(recvline));
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
		write(sockfd, sendline, n);
		}
	}
}
