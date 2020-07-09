/*      (C)2000 FEUP  */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "clientTCP.h"

#define SIZE_SERVER_RESP 200



int receive_from_server(int socket_fd , char * server_resp  ){


	int bytes;
	char buff[1];
	int i = 0; 

	while((bytes = read(socket_fd , buff ,1)) > 0){

		//printf("ciclo de leitura\n");
		
		if(buff[0] == '\n'){
			printf("\n");
			return bytes;	
		} 

		//if(buff[0] == '2' || buff[0] == '0' || buff[0] == '3' || buff[0] == '1' || buff[0] == '5'){

			printf("%s", buff );

			server_resp[i] = buff[0];
			i++;
			//if(i == 3) return bytes;
		//}
	}
	printf("\n");
	return bytes;	

}

int send_to_server(int sockfd , char * control_msg , int size){

	int bytes;

	bytes = write(sockfd, control_msg, size);


	return bytes;
}

int connect_to_server(char *IP , int port){

	int	sockfd;
	struct	sockaddr_in server_addr;

	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP);	//32 bit Internet address network byte ordered
	server_addr.sin_port = htons(port);		//server TCP port must be network byte ordered 
    
	//open an TCP socket
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    		perror("socket()");
        	exit(0);
    	}
	//connect to the server
    	if(connect(sockfd, 
	           (struct sockaddr *)&server_addr, 
		   sizeof(server_addr)) < 0){
        	perror("connect()");
		exit(0);
	}

	return sockfd;

}


int login_into_server(int sockfd , char * user_name , char * user_pass){
	int bytes;
	char user[40] = "USER";
	char pass[40] = "PASS";
	char space[] = " ";
	char new_line[] = "\r\n";
	char server_resp[SIZE_SERVER_RESP];

	strcat(user,space);
	strcat(user,user_name);
	strcat(user,new_line);
	printf("%s" , user);
	
	strcat(pass,space);
	strcat(pass,user_pass);
	strcat(pass,new_line);
	printf("%s\n" , pass);


	//se correto servidor responde 331 password required for...
	bytes = send_to_server( sockfd , user , strlen(user) );
	//printf("Bytes escritos %d\n", bytes);

	bytes = receive_from_server( sockfd , server_resp );
	//printf("Bytes lidos %d\n", bytes);

	//se correto servidor responde 230 user logged in...
	bytes = send_to_server( sockfd , pass , strlen(pass) );
	//printf("Bytes escritos %d\n", bytes);

	bytes = receive_from_server( sockfd , server_resp );
	//printf("Bytes lidos %d\n", bytes);
	if(atoi(server_resp) == 230){
		//printf("user logged in...\n");
		return 0;
	}

	
}

int send_pasv(int sockfd){

	int bytes;
	char pasv[10] = "PASV";
	char new_line[] = "\r\n";
	char server_resp[SIZE_SERVER_RESP ];
	int new_port = 0;
	int port_a; // parametro multiplicado por 256
	int port_b;
	

	strcat(pasv,new_line);
	bytes = send_to_server(sockfd , pasv , strlen(pasv));
	bytes = receive_from_server(sockfd , server_resp);
	
	/*for(int i = 0 ; i < strlen(server_resp) ; i++){

		printf("%d:%c\n", i , server_resp[i] );

	}*/

	//printf("%c\n", server_resp[43] );
	//printf("%c\n", server_resp[45] );

	
	port_a = server_resp[43] - '0';

	if(server_resp[46] == ')'){
		port_b = server_resp[45] - '0';
	}
	else{
		port_b = (server_resp[45] - '0')*10;
		port_b += server_resp[46] - '0';	
	}

	//printf("%d\n", port_a );
	//printf("%d\n", port_b );

	new_port = port_a*256 + port_b;
	//printf("%d\n", new_port );

	return new_port;



}

int send_list(int sockfd){

	int bytes;
	char list[10] = "LIST";
	char new_line[] = "\r\n";
	char server_resp[SIZE_SERVER_RESP ];


	strcat(list,new_line);
	bytes = send_to_server(sockfd , list , strlen(list));
	bytes = receive_from_server(sockfd , server_resp);


	return bytes;

}

int get_list(int sockfd ){

	int bytes;
	char buff[1];
	int i = 0; 

	while((bytes = read(sockfd , buff ,1)) > 0){

		printf("%s", buff );

		//server_resp[i] = buff[0];
		//i++;
			
	}
	printf("\n");
	return bytes;	

}

int get_file(int sockfd , char * file_name ){

	int bytes;
	char buff[1];
	int i = 0 , fd;

	fd = open(file_name , O_CREAT | O_RDWR | S_IXUSR );

	while((bytes = read(sockfd , buff ,1)) > 0){

		printf("%s", buff );
		write(fd , buff , 1);

		//server_resp[i] = buff[0];
		//i++;
			
	}
	printf("\n");
	return bytes;	

}


int send_retr(int sockfd ,  char * file_name){

	int bytes;
	char ret[100] = "RETR";
	char space[] = " ";
	char new_line[] = "\r\n";
	char server_resp[SIZE_SERVER_RESP ];


	strcat(ret,space);
	strcat(ret,file_name);
	strcat(ret,new_line);
	printf("%s" , ret);

	bytes = send_to_server(sockfd , ret , strlen(ret));
	//bytes = receive_from_server(sockfd , server_resp);

	return bytes;



}


/*
int main(int argc, char** argv){

	int	sockfd;
	struct	sockaddr_in server_addr;
	//char	buf[] = "Mensagem de teste na travessia da pilha TCP/IP\n";
	char server_resp[3];
	//char user_name[] = "USER demo\n";
	//char user_pass[] = "PASS password\n";
	int	bytes, i = 0;

	getip("test.rebex.net");
	
	

	//server address handling
	
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP);	//32 bit Internet address network byte ordered
	server_addr.sin_port = htons(FTP_PORT);		//server TCP port must be network byte ordered 
    
	//open an TCP socket
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    		perror("socket()");
        	exit(0);
    	}

    printf("Trying IP : %s on Port %d.....\n", IP , FTP_PORT);
	//connect to the server
    	if(connect(sockfd, 
	           (struct sockaddr *)&server_addr, 
		   sizeof(server_addr)) < 0){
        	perror("connect()");
		exit(0);
	}


	bytes = receive_from_server(sockfd , server_resp);

	printf("\nBytes lidos %d\nserver_resp : %s \n", bytes , server_resp);
    //send a string to the server
	
	printf("\n***** %s ******\n", user_name );
	bytes = write(sockfd, user_name, strlen(user_name));
	printf("Bytes escritos %d\n", bytes);

	receive_from_server(sockfd , server_resp);
	printf("\nBytes lidos %d\n  server_resp : %s \n", bytes , server_resp);

	/*bytes = write(sockfd, user_pass, strlen(user_pass));
	printf("Bytes escritos %d\n", bytes);
	
	receive_from_server(sockfd , server_resp);	
	printf("Bytes lidos %d\n", bytes);
	
	close(sockfd);
	exit(0);
}
*/

