

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "linklayer.h"
#include "time.h"


#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define SIZE 500
#define CDATA1 0x01
#define CDATA0 0x00
#define START 0x02
#define END 0x03
#define T1 0x00
#define T2 0x01
#define escape 0x7D
#define F 0x7E
#define A1  0x03 //endereco comandos enviados pelo E e Resp enviadas pelo Receptor
#define A2  0x01 //endereco comandos enviados pelo R e Resp enviadas pelo Emissor
#define SET 0x03
#define DISC 0x0A
#define UA 0x07
#define REJ0 0x01
#define REJ1 0x81
#define RR0 0x05
#define RR1 0x85


unsigned char C1_0 = 0 ;
volatile int STOP=FALSE;

struct termios oldtio,newtio;
clock_t start_t, end_t , total_t;
double time_taken;

//function used to send control frames
int sendcontrol(int fd,char A,char C){

  int wr;
  unsigned char buffer[5];


  buffer[0] = F ;
  buffer[1] = A ;
  buffer[2] = C ;
  buffer[3] = buffer[1]^buffer[2]; //BCC
  buffer[4] = F ;

  wr = write(fd, buffer, 5);
  if(wr < 0) return -1;

  return 0;


}
// function used to receive control frames
int receivecontrol(int port , char * c_frame){

  int rd,i=0,end_stream = 0,S0=1,S1=0,S2=0,S3 = 0, end = 0;

  unsigned char buffer;
  
  printf("waiting for return frame:\n");
  while(end_stream == 0)
  {
    
    rd = read(port , &buffer , 1 );

    if(buffer == F && S0 == 1)
    {
      i=0;
      c_frame[i] = buffer;
      i++;
      S0 = 0;
      S1 = 1;
    }

    if(buffer != F && S1 == 1 && S0==0)
    {

      c_frame[i] = buffer;
      i++;

    }

    if (i==4 && buffer == F)
    {

      c_frame[i] = buffer;
      end_stream = 1;
      S1 = 0;

    }

  }

  return 1;

}

// function returns 0 in case of error , 1 in case of success
// Transmitter llopen - sends the control frame SET , if true,
// wait to receive UA.
int llopen(int port, int Tx_Rx)
{
    unsigned char c_frame[5];
    int count = 0;
    int fd,c, res;
    unsigned char buf[255];
    int i, sum = 0, speed = 0;
    unsigned char portname[20] =  "/dev/ttyS";
    unsigned char portnumber[12];
    sprintf(portnumber, "%d", port);
    strcat(portname , portnumber);


    fd = open(portname, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(portname); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    printf("llopen inicio\n");


    if(Tx_Rx == 1)
    {

      if(sendcontrol(fd,A1,SET) < 0 )
      {
        printf("Failed to send SET control frame.\n");
        return 0;
      }

   
      while(count < 3)
      {
        receivecontrol(fd,c_frame);
        for (int j = 0 ; j< 5 ; j++)
        {
            printf("%x",c_frame[j]);
        }
        printf("\n");
        if(c_frame[3] == c_frame[1]^c_frame[2] && c_frame[2] == UA)
        {
            //printf("The connection was successfully established.\n");
            return fd;

        }
        c_frame[0] =0 ;
        count++;
      }

      //tried 3 times and failed
      return -1;

    }

    if(Tx_Rx == 2)
    {
      while(1)
      {

        receivecontrol(fd,c_frame);

        if( c_frame[3] == A1^SET && c_frame[2] == SET)
        { 

          //printf("vou enviar o UA: \n");

              
          if(sendcontrol(fd,A1,UA) == 0)
          {

            //printf("The SET frame was received , UA was sent.\n");
            return fd;
          }
        }
          
      }

    }



}

//used to send information frames
// returns number o written characters , negative value in case of error
int llwrite(int fd ,unsigned char *buffer , int length)
{
    int i , buffer_index , wr;
    unsigned char frameinfo[6+(length*2)] ,c_frame[5], aux_bcc2 ;
		
	
  

	//tcflush(fd , TCIOFLUSH);
	memset(frameinfo , 0 , sizeof(frameinfo));
	
	if(buffer[1] == 0 ) C1_0 = 0;
    //calculo do bcc2
    for(i = 0 ; i < length ; i++)
    {
      if(i == 0 ) aux_bcc2 = buffer[i];

      else
      {
        aux_bcc2 = aux_bcc2 ^ buffer[i];
      }


    }

    //printf("calculou bbc2 %x\nlength: %d\n", aux_bcc2, length );
    i = 0;
    frameinfo[0] = F;
    frameinfo[1] = A1; 
    frameinfo[2] = C1_0;
    frameinfo[3] = frameinfo[1] ^ frameinfo[2];
    i = 4;
    for( buffer_index = 0 ; buffer_index < length ; buffer_index++)
    {
      if(buffer[buffer_index] == F || buffer[buffer_index] == escape)
      {

        frameinfo[i++] = escape;
        frameinfo[i++] = buffer[buffer_index] ^ 0x20;
      }
      else
      {
        frameinfo[i++] = buffer[buffer_index];
      }

    }
	if(aux_bcc2 == escape || aux_bcc2 == F)
	{
		frameinfo[i++] = escape ;
		frameinfo[i++] = aux_bcc2 ^ 0x20 ;
	}
	else
	{
		frameinfo[i++] = aux_bcc2 ;//BCC2
	}
    
    frameinfo[i++] = F;

    /*for(int k = 0 ; k < i; k++)
    {
      printf("%x", frameinfo[k] );
    }

    printf("\n");*/

    envia:
	
    sendagain : wr = write(fd , frameinfo , i);
    if(wr < 0) return -1;


    //printf("ja enviei o frame \n");
    //printf(" aux_bcc2 : %x\n", aux_bcc2 );

	 
	  //usleep(200000);

    while(1)
    {
	  
	  
      printf("waiting : control frame (REJ ou RR)\n");
	  
      

		  if( (((double)(end_t-start_t))/CLOCKS_PER_SEC) > 3  )
      {
        goto envia;
        break;
      }    
      receivecontrol(fd , c_frame );

      for(int j = 0 ; j < 5 ; j++)
      {
        printf("%x", c_frame[j] );
      }

      printf("\n");

      if( ( c_frame[3] == c_frame[1]^c_frame[2] ) && (c_frame[2] == RR0 || c_frame[2] == RR1 ) )
      {
        if(c_frame[2] == RR0) 
		{
			printf("Recebeu RR0 : %x\n", RR0 );
			C1_0 = 0;
		}
		
        if(c_frame[2] == RR1) 
		{
			printf("Recebeu RR1 : %x\n", RR1 );
			C1_0 = 1;
		}

        return fd ;
      }

      else if( ( c_frame[3] == c_frame[1]^c_frame[2] ) && (c_frame[2] == REJ1 || c_frame[2] == REJ0 ) )
      {
         if(c_frame[2] == REJ0) printf("Frame foi rejeitado : %x\n", REJ0 );
         if(c_frame[2] == REJ1) printf(" Frame foi rejeitado : %x\n", REJ1 );

         goto envia;
      }


    }


}

//llread recebe os frames e envia os pacotes para a camda de aplicação

int llread(int fd ,unsigned char *package )
{
  int rd, end_stream = 0 , S0 = 1 , S1 = 0 , S2 = 0 , S3 = 0 , n_data;
  unsigned char buffer ,header[4], aux_bcc2 , aux;
  unsigned char * prepackage = malloc(SIZE*sizeof(char));
  unsigned char *  auxpck = malloc(SIZE * sizeof(char));
  int i = 0 , p = 0,l ;
  //printf("Cheguei ao llread\n");

  printf("a receber frame :\n");


  tcflush(fd, TCIOFLUSH);

  //memset(auxpck , 0 , sizeof(auxpck));
  //memset(prepackage , 0 , sizeof(prepackage));

  while(end_stream == 0){
 	


	
			
    rd = read( fd , &buffer , 1 );
	
    if(buffer == F && S0 == 1)
    {
      header[i] = buffer;
      i++;
      S0 = 0;
      S1 = 1;
    }

    if(buffer != F && S1 == 1 && S0==0)
    {


      if(i == 3 && buffer == header[1] ^ header[2] )
      {
        i = 0 ;
        S1 = 0 ;
        S2 = 1 ;
        continue;

      }

       if(i == 3 && S2 == 0 && buffer != header[1]^header[2]  )
      {
        if(header[2] == CDATA0) sendcontrol(fd , A2 , REJ1 );
        else if (header[2] == CDATA1) sendcontrol(fd , A2 , REJ0);
        end_stream =1 ;
        break;

      }

      header[i] = buffer;
      i++;
    }

    //lê a informação do package e faz o byte destuffing
    if(S2 == 1 && S0 == 0 && S1 == 0)
    {

     

      if(buffer != F && S3 == 0 && buffer != escape)
      {
          prepackage[p] = buffer;
          p++;

      }

     else if(buffer == F)
      {

        S2 = 0;
        end_stream = 1;
        break;

      }

       else if(buffer == escape && S3 == 0)
      {
        S3 = 1;
        continue;
      }

      else if(S3 == 1)
      {
        prepackage[p] = buffer ^ 0x20;
        p++;
        S3 = 0;
      }

    }

  }

  n_data = p;
  p =0;
  aux_bcc2 = prepackage[0];
  for(int i = 0 ; i < n_data ; i++)
  {
    if(i == (n_data-1))
    {
      aux = prepackage[i];
    }
    //printf("%x", prepackage[i] );

  }

  //printf("\n" );

  //printf("acabou de ler o frame \n");
  //verificar o bcc
  //printf("tamanho prepackage %d\n" , n_data);

  for(l = 0 ; l < n_data-1 ; l++)
  {
		
    if(l == 0)
	{
	 aux_bcc2 = prepackage[l];
	//printf("valor de aux_bcc2 : %x\n",aux_bcc2);
	}
    else
    {

      aux_bcc2 = aux_bcc2 ^ prepackage[l];
	  //printf("valor de aux_bcc2 : %x\n",aux_bcc2);
    }

      auxpck[l] = prepackage[l];

      //printf("%x\n", auxpck[l]);
  }

  //printf("\n" );

  //printf(" aux: %x , bcc2 : %x\n", aux , aux_bcc2);
  //printf("ja verificou bcc\n");



  
  if( aux_bcc2 != aux )
  {
  

    printf("frame was rejected\n");
    if(header[2] == CDATA0) sendcontrol(fd , A2 , REJ1 );
    else if (header[2] == CDATA1) sendcontrol(fd , A2 , REJ0);
    
    return 0;
  }

  else if(aux_bcc2 == aux )
  {
	
    //printf("Pacote a ser eviado para a APP layer\n" );
    for(int f = 0 ; f < n_data-1 ; f++)
    {
      package[f] = auxpck[f];
      //printf("%x", package[f] );
    }


    //printf("\n" );
    //printf("frame ACK\n");

    if(header[2] == CDATA0) sendcontrol(fd , A2 , RR1 );
    else if (header[2] == CDATA1) sendcontrol(fd , A2 , RR0);

    return n_data-1;
  }






}


int llclose(int fd , int Tx_Rx)
{
  char c_frame[5];
  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1)
  {
      perror("tcsetattr");
      exit(-1);
  }

 
  if(Tx_Rx == 1)
  {
	sendcontrol(fd , A1 , DISC);	
	while(1)
    { 
    receivecontrol(fd,c_frame);

    if( c_frame[3] == c_frame[1]^c_frame[2]  && c_frame[2] == DISC )
    {
        printf("Connection Closed\n");
        sendcontrol(fd , A1 , UA);
        close(fd);
        return 1;
    }

   }
  }
  if(Tx_Rx == 2)
  {
	while(1)
    { 
    receivecontrol(fd,c_frame);

    if( c_frame[3] == c_frame[1]^c_frame[2]  && c_frame[2] == DISC )
    {
        printf("Connection Closed\n");
        sendcontrol(fd , A2 , DISC);
        close(fd);
        return 1;
    }

  	}
  




  }
}

