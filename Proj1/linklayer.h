int sendcontrol(int fd,char A,char C);

int receivecontrol(int fd , char * c_frame);

int llopen(int port, int Tx_Rx);

int llwrite(int port , unsigned char *buffer , int length);

int llread(int fd , unsigned char *package );

int llclose(int fd, int Tx_Rx);
