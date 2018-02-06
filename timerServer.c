/**
* Burak Demirci
* 141044091
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/time.h> 
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>  
#include <sys/wait.h>
#include <string.h>

#define BUFSIZE 2048
#define FIFO_PERM  (S_IRUSR | S_IWUSR)
#define MS 1000
#define LOGNAME "log/timerServer.log"
typedef struct node 
{
	int pidN;
	struct node *next;
}node_t;

void removeList(node_t *head,int val);
ssize_t r_read(int fd, void *buf, size_t size);
ssize_t r_write(int fd, void *buf, size_t size);
double Determinant(double **a,int n);
double **randoMatrix(int matrixsize);

int main(int argc, char const *argv[])
{
    FILE *out;/*Log dosyası icin file pointer*/
	node_t *List;
	int i,j;
	struct timeval start ,stop;
	double elapsedTime;
	int matrixSize = atoi(argv[2])*2;
	int mainfifo;/* Main request fifo*/
	int matrisSentfifo; /*Sent to matrix client fifo*/
	pid_t Forkpid;
	double determinant;
	char ForkFifo[BUFSIZE];
	char buf[BUFSIZE];
	char tempStr[BUFSIZE];
	char martixsent[BUFSIZE];
	double **matrix;
    int clientPid;
	int deneme[1];

	if (argc !=4)
	{
		printf("Wrong usage : ./timeServer <ticsinms> <n> <mainpipename> \n");
		return -1;
	}
	if (mkfifo(argv[3], FIFO_PERM) == -1) 
	{
		/*Create main fifo*/
        if (errno != EEXIST)
        {
        	printf("failed to create named pipe %s\n", argv[3]);
        	return 1; 
      	}
   	}
    /*Server ana dongusu */
   	while(1)
   	{
   		while (((mainfifo = open(argv[3], O_RDONLY)) == -1) && (errno == EINTR)) ; 
   		if (mainfifo == -1) {
      		printf("failed to open named pipe %s\n",argv[3]);
     		return 1; 
   		}
		r_read(mainfifo,deneme,sizeof(int));/*Read cliend pid =  Request*/
   		close(mainfifo);
   		sprintf(buf,"%d",deneme[0]);
        clientPid = deneme[0];
   		printf("PID %d*\n",deneme[0] );
   		/*----------------------------------------------*/
   		Forkpid = fork();
   		if( Forkpid == 0 )
   		{
   			sprintf(ForkFifo,"%d",deneme[0]);
   			printf("----> *%s*\n",ForkFifo );
   			while(1)
   			{
   				/*Matrixi cliente gondere islemi */
   			   	while (((matrisSentfifo = open(ForkFifo, O_WRONLY)) == -1) && (errno == EINTR)) ; 
   			   	if (matrisSentfifo == -1) 
   			   	{
   			     	printf("failed to open named pipe %s\n",ForkFifo);
   			    	return 1; 
   			   	}
   			   	
   			   	printf("FORK\n");
                /*Elapsed time hesaplamak için baslangic noktası */
   			   	gettimeofday(&start, NULL);  			   	
   			    do
   			    {

   					matrix=randoMatrix(matrixSize);

   					gettimeofday(&stop, NULL);
   				    determinant=Determinant(matrix,matrixSize);
   				    determinant=100;
   			   	}while(determinant==0.0);	
   			   	/*Log dosyasını acma islemi */
                out = fopen(LOGNAME,"a");

   			   	elapsedTime = (stop.tv_sec - start.tv_sec) * 1000.0;   
    			elapsedTime += (stop.tv_usec - start.tv_usec) / 1000.0; 
   			   	printf("Pid: %d ElapsedTime: %f Determinat: %f\n",clientPid,elapsedTime,determinant);
   			   	fprintf(out,"Pid: %d ElapsedTime: %f Determinat: %f\n",clientPid,elapsedTime,determinant);
   			   	i=0;
   			   	j=0;
   			   	sprintf(buf," ");
   			   	while(i< matrixSize)
   			   	{
   			   		while(j < matrixSize)
   			   		{	
   			   			sprintf(tempStr,"%.0f ",matrix[i][j]);
   			   			strcat(buf,tempStr);	
   			   			j++;
   			   		}
   			   		j=0;	
   			   		i++;
   			   	}
   			   	printf("%s\n",buf);
   			   	r_write(matrisSentfifo,buf,strlen(buf)+1);			
   			   	close(matrisSentfifo);

   			   	while (((matrisSentfifo = open(ForkFifo, O_RDONLY)) == -1) && (errno == EINTR)) ; 
   			   	if (matrisSentfifo == -1) 
   			   	{
   			     	printf("failed to open named pipe %s\n",ForkFifo);
   			    	return 1; 
   			   	}
   			   	r_read(matrisSentfifo,deneme,sizeof(int));/*Read cliend pid =  Request*/
   			   	close(matrisSentfifo); 
   			   	fclose(out);
                /*Free memory */
                for (i = 0; i < matrixSize; ++i) free(matrix[i]);
                free(matrix);
                usleep(atoi(argv[1])*MS);
   			}
   			exit(0);
   		}
   		/*Server belli bir sure bekliyor*/
   		usleep(atoi(argv[1])*MS);/*MS ile çarp*/
   	}
   	wait(NULL);
   	
   	/*Main fifoyu kapatma ve silme islemi*
    close(argv[3]);
 */
	return 0;
}

double **randoMatrix(int matrixsize)
{
	/*Ikı boyutta arraye matris uretme fonksiyonu */
	double **matrix = (double **)calloc(matrixsize,sizeof(double));
	int i=0,j=0;
	srand(time(NULL));
	while(i< matrixsize)
	{
		matrix[i] = (double*)calloc(matrixsize,sizeof(double));
		while(j < matrixsize)
		{	
			matrix[i][j] = rand()%15;
			j++;
		}
		j=0;	
		i++;
	}
	return matrix;
}

double Determinant(double **a,int n)
{
	/*Determinant fonksiyonu internetten*/
	/*http://paulbourke.net/miscellaneous/determinant/determinant.c*/
	/*Adresinden alınmıştır */
    int i,j,j1,j2 ;                    
    double det = 0 ;                  
    double **m = NULL ;                
    if (n < 1)    {   }                

    else if (n == 1) {                
        det = a[0][0] ;
        }

    else if (n == 2)  {                
                                       
        det = a[0][0] * a[1][1] - a[1][0] * a[0][1] ;
        }                       
    else {                             
                                       
        det = 0 ;                                                         
        for (j1 = 0 ; j1 < n ; j1++) {
                                           
            m = (double **) malloc((n-1)* sizeof(double *)) ;

            for (i = 0 ; i < n-1 ; i++)
                m[i] = (double *) malloc((n-1)* sizeof(double)) ;
                       
            for (i = 1 ; i < n ; i++) {
                j2 = 0 ;               
                for (j = 0 ; j < n ; j++) {
                    if (j == j1) continue ; 
					m[i-1][j2] = a[i][j] ;                         
                    j2++ ;                 
                }
            }
            det += pow(-1.0,1.0 + j1 + 1.0) * a[0][j1] * Determinant(m,n-1) ;
            for (i = 0 ; i < n-1 ; i++) free(m[i]) ;                         
            free(m) ;                                                       
        }
    }
    return(det) ;
}





void removeList(node_t *headN,int val)
{
	node_t *temp= NULL;
/*	while(headN!=NULL)
	{

		if (headN.pidN == val)
		{
			temp.next=head.next;
			head=NULL;
			return;
		}
		temp=headN;
		headN=headN.next;
	}	*/
}
ssize_t r_write(int fd, void *buf, size_t size) 
{
	/*Bu fonksiyon kitaptan alınmıştır*/
   char *bufp;
   size_t bytestowrite;
   ssize_t byteswritten;
   size_t totalbytes;

   for (bufp = buf, bytestowrite = size, totalbytes = 0;
        bytestowrite > 0;
        bufp += byteswritten, bytestowrite -= byteswritten) {
      byteswritten = write(fd, bufp, bytestowrite);
      if ((byteswritten) == -1 && (errno != EINTR))
         return -1;
      if (byteswritten == -1)
         byteswritten = 0;
      totalbytes += byteswritten;
   }
   return totalbytes;
}
ssize_t r_read(int fd, void *buf, size_t size) 
{
	/*Bu fonksiyon kitaptan alınmıştır*/
   ssize_t retval;
   while (retval = read(fd, buf, size), retval == -1 && errno == EINTR) ;
   return retval;
}
