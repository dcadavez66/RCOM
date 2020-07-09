#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
//#include <strings.h>
#include <string.h>
#include "clientTCP.h"

#define FTP_PORT 21
#define SIZE_SERVER_RESP 200


char *IP;

void getip(char hostname[])
{
	struct hostent *h;

        if ((h=gethostbyname(hostname)) == NULL) {  
            herror("gethostbyname");
            exit(1);
        }

        printf("Host name  : %s\n", h->h_name);
        IP = inet_ntoa(*((struct in_addr *)h->h_addr));
        //printf("IP Address : %s\n",IP);

    
}


int main (int argc , char * argv[])
{
	int sockfd , sockfd_2 , bytes;
	char * server_name = malloc(20*sizeof(char));
	char * user_name = malloc(20*sizeof(char));
	char * user_pass = malloc(30*sizeof(char));
	char * file_name = malloc(40*sizeof(char));
	char server_resp[SIZE_SERVER_RESP];
	int new_port;
	
	
	printf("**************************\n*     FTP Client         *\n**************************\n");


	printf("Server name:");
	scanf("%s", server_name);
	getip(server_name);
	printf("connecting to %s...\n",IP );

	sockfd = connect_to_server( IP ,FTP_PORT );
	//printf("%d\n" , sockfd);

	bytes = receive_from_server(sockfd , server_resp);
	//printf("\nReceived bytes: %d\n" , bytes);

	printf("(USER):");
	scanf("%s", user_name);
	printf("(PASS):");
	scanf("%s", user_pass);

	//printf("%s\n",user_pass );

	if(login_into_server(sockfd , user_name ,user_pass) == 0 ){

		//user logged in
	}
	else{
		printf("failed to log in\n");
		exit(0);
	}

	 new_port = send_pasv(sockfd);
	 printf("%d\n", new_port );

	 //printf("%s\n", IP );
	 sockfd_2 = connect_to_server(IP , new_port);

	 bytes = send_list(sockfd);

	 bytes = get_list(sockfd_2);

	 bytes = receive_from_server(sockfd , server_resp);

	 close(sockfd_2);

	 printf("Choose the file you want to transfer:");
	 scanf("%s", file_name);

	 new_port = 0;
	 new_port = send_pasv(sockfd);

	 //printf("%d\n", new_port );

	 //printf("%s\n", IP );
	 sockfd_2 = connect_to_server(IP , new_port);


	 send_retr(sockfd , file_name);

	 bytes = get_file(sockfd_2, file_name);

	 close(sockfd_2);

	 bytes = receive_from_server(sockfd , server_resp);







}