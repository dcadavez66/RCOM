#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "apppck.h"

int control_package(char C , char T1 , int L1 , unsigned char *V1 , char T2 , int L2 ,unsigned char *V2,unsigned char * buff)
{
	//printf("entrei no control_package\n");
	//printf("fsize: %s \n filename: %s\n",V1 ,V2);
	//printf("tamanho fsize : %d(%d) \n tamanho filename : %d(%d)\n" , strlen(V1), L1, strlen(V2), L2);
	int sizepck = 0,j, v1_index ,v2_index;

	buff[0] = C;
	//printf("pos(%d)%x\n ",i , buff[0]);
	buff[1] = T1;
	//printf("pos(%d)%x\n ",i , buff[1]);
	buff[2] = L1;
	//printf("pos(%d)%x\n ",i , buff[2]);
	sizepck= 3;
	for( v1_index = 0 ; v1_index < L1 ; v1_index++)
	{
		buff[sizepck] = V1[v1_index];
		//printf("pos(%d)%x\n ",i , buff[i]);
		sizepck++;

	}

	buff[8] = T2;
	//printf("pos(%d)%x\n ",i++ , buff[8]);
	buff[9] = L2;
	//printf("pos(%d)%x\n ",i++ , buff[9]);
	sizepck= 10;
	for(v2_index = 0 ; v2_index < L2; v2_index++)
	{
		buff[sizepck] = V2[v2_index];
		//printf("pos(%d)%x \n",i , buff[i]);
		sizepck++;
	}
	return sizepck;
}

int  data_package(char C , char N ,int k, unsigned char * databuff ,unsigned char * buff)
{
	int sizepck = 0 , data_index = 0 ;
	
	//memset(buff, 0 , sizeof(buff));

	buff[0] = C;
	buff[1] = N;
	buff[2] = k/256;
	buff[3] = k%256;
	sizepck = 4;
	for(data_index = 0 ; data_index < k ; data_index++)
	{
		buff[sizepck] = databuff[data_index];
		sizepck++;
	}
	return sizepck;
}
