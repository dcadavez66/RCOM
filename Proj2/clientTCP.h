int connect_to_server(char *IP , int port);

int send_to_server(int sockfd , char * control_msg , int size);

int receive_from_server(int socket_fd , char * server_resp  );

int login_into_server(int sockfd , char * user_name , char * user_pass);

int send_pasv(int sockfd);

int send_list(int sockfd);

int get_list(int sockfd);

int send_retr(int sockfd ,  char * file_name);

int get_file(int sockfd , char * file_name );