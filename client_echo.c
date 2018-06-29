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
#include	<sys/select.h>

#define	max(a,b)	((a) > (b) ? (a) : (b))
#define MAXLINE 500

void str_cli(int, int);

int
main(int argc, char **argv)
{
	int	sockfd;
	struct sockaddr_in	servaddr;
	int fd;


	if (argc != 4)
		{
		printf("usage: client <IPaddress> <Port> <source_donnÃ©es>\n");
		exit(-1);
		}

	// crÃ©ation de la socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// mise Ã  zÃ©ro de la structure servaddr
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET; // IPv4
	// l'argument nÂ°2 est converti en entier, mis en ordre rÃ©seau 
	// et affectÃ© au champ sin_port
	servaddr.sin_port = htons(atoi(argv[2]));
	// L'argument nÂ°1 (l'adresse IP sous forme dÃ©cimale pointÃ©e)
	// est converti en forme numÃ©rique et affectÃ© au champ sin_addr
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	// Demande de connexion au serveur
	connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

	// Si l'argument nÂ°3 n'est pas la chaÃ®ne "stdin" 
	if (strcmp(argv[3],"stdin")!=0) 
		{
		// alors l'argument nÂ°3 est un nom de fichier que l'on ouvre
		// le descripteur retournÃ© par open est mis dans fd
		fd=open(argv[3],O_RDONLY);
		if (fd<0)	// On teste si l'ouverture du fichier s'est bine passÃ©e.
			{
			printf("Erreur d'ouverture du fichier %s\n",argv[3]);
			exit(-1);
			}
		}
	else fd=STDIN_FILENO; // Sinon fd prend le descripteur de stdin

	str_cli(fd, sockfd);

	exit(0);
}

void
str_cli(int fd, int sockfd)
{
	int		maxfdp1;
	fd_set		rset;
	char		sendline[MAXLINE], recvline[MAXLINE];
	int 		n;

	FD_ZERO(&rset); // On met Ã  zÃ©ro l'ensemble rset
	for ( ; ; ) {
		FD_SET(fd, &rset); // On met dans rset le descripteur de l'entrÃ©e de donnÃ©es
		FD_SET(sockfd, &rset); // On met dans rset le descripteur de socket
		maxfdp1 = max(fd, sockfd) + 1; // on calcule le maximum est deux descripteurs Ã  surveiller et on ajoute 1
		// on se met en attente d'un Ã©vÃ¨nement 
		// sur l'entrÃ©e de donnÃ©e ou la socket
		select(maxfdp1, &rset, NULL, NULL, NULL);
		// aprÃ¨s select : un Ã©vÃ¨nement s'est produit au moins un des deux descripteurs surveillÃ©s

		// Si le descripteur de socket appartient Ã  rset aprÃ¨s select
		// alors c'est qu'il y a eu un Ã©vÃ¨nement sur la socket
		if (FD_ISSET(sockfd, &rset))
			{
			// On lit sur la socket
			// Si la valeur de retour de read est nulle
			// C'est que le serveur a envoyÃ© un segment FIN de dÃ©connexion.
			if ((n=read(sockfd, recvline, MAXLINE)) == 0)
				{
				printf("str_cli: serveur termine prematurement\n");
				exit(-1); // on termine le client
				}
			else if (n<0) // Si read retourne une valeur nÃ©gative, c'est qu'un segment RST a Ã©tÃ© reÃ§u et qu'il y a une erreur sur socketla
					{
					printf("Erreur de socket\n");
					exit(-1); // on termine le client
					}
			// Sinon, read a retournÃ© une valeur strictement positive
			// C'est-Ã -dire que des donnÃ©es ont Ã©tÃ© reÃ§ues
			write(STDOUT_FILENO,recvline,n); // On affiche ces donnÃ©es
			if (fd==STDIN_FILENO) printf("\nentrez votre chaine :\n");

		}

		// Si le descripteur de l'entrÃ©e de donnÃ©es appartient Ã  rset aprÃ¨s select
		// alors c'est qu'il y a eu un Ã©vÃ¨nement sur l'entrÃ©e de donnÃ©es
		if (FD_ISSET(fd, &rset)) {
			// On lit sur l'entrÃ©e de donnÃ©es
			// Si la valeur de retour de read est nulle
			// C'est qu'on a atteint la fin de fichier
			// ou que l'utilisateur a fini la saisie.
			if ((n=read(fd, sendline, MAXLINE)) == 0)
				{
				close(sockfd); // On ferme la socket
				exit(0); // On termine le client
				}
			// sinon on Ã©crit dans la socket les n caractÃ¨res
			// lus au clavier et stockÃ©s dans sendline
			write(sockfd, sendline, n);
		}
	}


}