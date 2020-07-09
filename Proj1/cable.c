#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "linklayer.h"
#include "apppck.h"

#define CDATA0 0x00
#define CDATA1 0x01
#define START 0x02
#define END 0x03
#define T1 0x00
#define T2 0x01
#define SIZE 500
#define BUFF_SIZE 500
#define frsize 20

unsigned char * filestr;

int main (int argc , char * argv[])
{
	int fd, rd, wr , port,input , N , L2 , L1 , k , ncontrolpck ,i,j,data_size;
	unsigned char * filename = malloc(100*sizeof(char));
	unsigned char fileinfo[SIZE];
	unsigned char fsize[15];
	unsigned char * databuffer = malloc(frsize*sizeof(char));
	unsigned char * package = malloc(SIZE * sizeof(char));;
	unsigned char * realdata = malloc(sizeof(char));
	
	int filesize;
	FILE *fptr;
	struct stat buffer;
	int status;



	printf("Choose the port you want to use :  ");

	scanf("%d",&port);

	printf("\nChose the transmitter or the receiver:\n");
	jump:printf("  1-Transmitter   2 -Receiver  3-Exit \n");

	 printf("enter your option:");

	fflush(stdin);
	scanf("%d", &input);
	
	/********************************************************/
   /*              Transmissor                              /
  /********************************************************/

	if(input == 1)
	{
		printf("Welcome to the transmitter here you may chose the file you want to send.\n");
		printf("file name : ");

		fflush(stdin);
		scanf("%s",filename);
		

		printf("%s\n", filename );

		fptr = fopen(filename , "rb");

		if(fptr == NULL)
		{
			printf("Error !\n");
			exit(1);
		}

		status = stat(filename , &buffer);
		if(status == 0)
		{
			filesize = buffer.st_size;
		}

		printf("file size: %d bytes\n",filesize );
		filestr = malloc(filesize*sizeof(char));
		fread(filestr , 1 , filesize , fptr);

		fd = llopen(port , input);

		if( fd < -1 )
		{
			printf("error connecting to the serial port\n");
			exit(1);
		}

		printf("The connection was successfully established.\n");
		//send first control package
		sprintf(fsize, "%d", filesize);

		//printf("%s\n", fsize );
		int a, b;
		a = strlen(fsize);
		b = strlen(filename);
		filename = realloc(filename , b* sizeof(char));

		//printf("valor do fsize: %d %s\n" , a, fsize);
		//printf("valor do filename: %d \n" , b);



		ncontrolpck = control_package(START , T1 ,a , fsize , T2 , b, filename , fileinfo);

		wr = llwrite(fd , fileinfo , ncontrolpck);
		if(wr < 0)
		{
			printf("error sending file info \n");
			exit(1);
		}

				
		for(int i = 0 ; i < filesize ; i = i+frsize)
		{	
			
		 
			if(filesize-i > frsize) k = frsize;
			else k = filesize-i;
			memcpy(databuffer, filestr+i , k );
			
			ncontrolpck = data_package( CDATA1 , (N++)%255 ,k, databuffer , package );

			package = realloc(package , ncontrolpck * sizeof(char));

			wr = llwrite(fd , package , ncontrolpck);
			
		}
		

		ncontrolpck = control_package(END , T1 , a , fsize , T2 , b , filename , fileinfo  );

		wr = llwrite(fd , fileinfo , ncontrolpck);
		if(wr < 0)
		{
			printf("error sending file info \n");
			exit(1);
		}

		llclose(fd, input);
		fclose(fptr);


	}
/********************************************************/
/*              RECETOR                                 */
/********************************************************/

	if(input == 2)
	{
		printf("Welcome to the Receiver , wait till your file arrives\n");

		fd = llopen(port, input);

		if (fd < 0){
          printf("Error Connecting");
          return -1;
        }



		i = 0;
		while(1)
		{	
			
			rd = llread( fd, package);
			package = realloc(package , rd *sizeof(char));

			/*for(int f = 0 ; f < rd ; f++)
			{
				printf("%x", package[f]);
			}

			printf("\n");*/

			if(package[0] == START)
			{
				//printf("recebi pacote de Start\n");


				if(package[1] == T1)
				{
					i=3;
					for( j = 0 ; j <= package[2]  ; j++)
					{
						char aux = package[i];
						if(j == 1)
						{
							
							filesize = filesize + atoi(&aux)+1 ;
							for(int ncasasdec = 1 ; ncasasdec < package[2]-j ; ncasasdec++)
							{
								filesize = filesize*10;
							}
							break;
						} 

						filesize = (filesize + atoi(&aux));
						i++;
					}

					i=10;
					filename = realloc(filename , package[10]*(sizeof(char)));
					for( k = 0 ; k< package[10] ; k++ )
					{

						filename[k] = package[i];
						i++;

					}

				}

				if(package[1] == T2)
				{
					i = 2;
					for( j = 0 ; j < package[2] ; j++)
					{
						i++;
						filename[j] = package[i];

					}

					i=10;

					for( j = 0 ; j <= package[2]  ; j++)
					{
						
						unsigned char aux = package[i];

						if(j == 1)
						{
							filesize = (filesize + (atoi(&aux)+1) );
							for(int ncasasdec = 1 ; ncasasdec < package[2]-j ; ncasasdec++)
							{
								filesize = filesize*10;
							}
							break;
						} 

						else filesize = (filesize +  atoi(&aux))*10;
						i++;
					}


				}
				
				
				
				//printf("Created a new file\n" );
				fptr = fopen(filename , "wb");
				if(fptr < 0 ) printf("failed to create file\n" );
				filestr = malloc(filesize*sizeof(char));

			}

			else if(package[0] == END)
			{
				//printf("recebi END\n");
				break;


			}

			else if(package[0] == CDATA1)
			{
				//printf("Recebi pacote de dados %d\n",package[1] );
				data_size = package[2]*256 + package[3];
								i = 4;
				for(int d = 0 ; d < data_size ; d++)
				{
					realdata[d] = package[i++];
				}

				fwrite( realdata, 1 , data_size , fptr);
				

			}

		}
		printf("File received!\n");
		llclose(fd ,input);
		fclose(fptr);

		
	}
	/********************************************************/
   /*              EXIT APP                                */
  /********************************************************/
	if(input == 3)
	{
		printf("Cable will close:\n");
		return 1;
		
	}

	else goto  jump;
}
