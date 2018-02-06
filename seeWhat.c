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

int mypid;

ssize_t r_write(int fd, void *buf, size_t size);
ssize_t r_read(int fd, void *buf, size_t size);
double Determinant(double **a,int n);
double **CompoundMatrix(double** m1,double** m2,double** m3,double** m4, int n );
double** Inverse(double** matrix, int n);
void SignalHandler(int signo);

int main(int argc, char const *argv[])
{
	FILE *out;
    int mainfifo; /*main fifo to server*/
	int matrixFifo; /*matrix fifo request matrix to server*/
	int showfifo; /*sent to result show client*/
	char showFifoName[BUFSIZE];
	int  showFifoData1,showFifoData2;/*Result1 ve result2 için fifo*/
	char showFifoDataName1[BUFSIZE];/*Result1 için fifo name*/
	char showFifoDataName2[BUFSIZE];/*Result2 için fifo name*/
	char matrisFifoName[BUFSIZE];/*Matrix fifo Name( pid )*/
	char buf[BUFSIZE];
	double result1[1],result2[1],Orjinaldet;
	pid_t clientPD;
	pid_t forkR1,forkR2;
    struct timeval startR1 ,stopR1,startR2,stopR2;/*Elapsed timı belirlemek icin kullanılan*/
    double elapsedTimeR1[1],elapsedTimeR2[1];    /*değiskenler*/
	double **matrix;
	char *token;
    char FileName[BUFSIZE];
	double tempMatrix[BUFSIZE];
	int matrixSize;
	char def[2];
  	int pid[1];
  	int i=0,j=0,k=0,n;
    double **R1matrix,**R2matrix;
    double **R1m1,**R1m2,**R1m3,**R1m4;
    double **R2m1,**R2m2,**R2m3,**R2m4;
	if (argc !=2)
	{
		printf("Wrong usage : ./seeWhat <pipename> \n");
		return -1;
	}
    signal (SIGINT, SignalHandler);	
	clientPD = getpid();
	sprintf(showFifoName,"showresult");
	
  	/*clien pid'yi atma islemi */
  	pid[0] = (int)clientPD;
    mypid=pid[0];
  	/*Matrixin istenecegi fifonun ismini belirleme*/
  	sprintf(matrisFifoName,"%d",(int)clientPD);
    /*Serverden matrixi isteyan fifoyu oluşturma */
  	if (mkfifo(matrisFifoName, FIFO_PERM) == -1) 
	{
		/*Create main fifo*/
    	if (errno != EEXIST)
    	{
    		printf("failed to create named pipe %s\n", matrisFifoName);
    		return 1; 
  		}
	}
    /*Log dosyasını olusturma islemi */
    sprintf(FileName,"log/%d.log",pid[0]);
    out = fopen(FileName,"a");
    /*Show resulta result1 bilgilerini yollamak icin olusturulan fifo*/
    sprintf(showFifoDataName1,"%ds1",pid[0]);
  	/*Show resulta result1 bilgilerini yollamak icin olusturulan fifo */
  	if (mkfifo(showFifoDataName1, FIFO_PERM) == -1) 
	{
		/*Create main fifo*/
    	if (errno != EEXIST)
    	{
    		printf("failed to create named pipe %s\n", showFifoDataName1);
    		return 1; 
  		}
	}

	/*Show resulta result2 bilgilerini yollamak icin olusturulan fifo*/
    sprintf(showFifoDataName2,"%ds2",pid[0]);
  	/*Show resulta result2 bilgilerini yollamak icin olusturulan fifo */
  	if (mkfifo(showFifoDataName2, FIFO_PERM) == -1) 
	{
		/*Create main fifo*/
    	if (errno != EEXIST)
    	{
    		printf("failed to create named pipe %s\n", showFifoDataName2);
    		return 1; 
  		}
	}
	/*Serverden request icin açılan fifo */
    while (((mainfifo = open(argv[1], O_WRONLY)) == -1) && (errno == EINTR)) ; 
    if (mainfifo == -1) {
      fprintf(stderr, "failed to open named pipe %s\n",argv[1]);
      return 1; 
    } 
    	
    /* Serverden request islemi */
   	r_write(mainfifo,pid,sizeof(int));
	close(mainfifo);
    printf("Serverden istek: %d*\n",pid[0]);
    while(1)
    {
    	/*Matrix siz'i sifirlama islemi*/
    	matrixSize=0;

    	/*Serverden matrixi isteyen fifoyu açma*/
    	while (((matrixFifo = open(matrisFifoName, O_RDONLY)) == -1) && (errno == EINTR)) ; 
    	if (matrixFifo == -1) {
    	  fprintf(stderr, "failed to open named pipe %s\n",matrisFifoName);
    	  return 1; 
    	} 
    	
    	r_read(matrixFifo,buf,BUFSIZE);
    	close(matrixFifo);
    	/*String seklinde gelen matrixi token islemi */
    	token = strtok(buf, " ");
    	sprintf(def,token);
    	tempMatrix[0]=atoi(def);
   		while( token != NULL ) 
   		{
   		   token = strtok(NULL, " ");
   		   sprintf(def,token);
   		   matrixSize++;
    	   tempMatrix[matrixSize]=atoi(def);
   		}
		matrixSize = sqrt(matrixSize);
		matrix = (double**)calloc(matrixSize,sizeof(double));
		k=0;
        fprintf(out, "[");
    	for (i = 0; i < matrixSize; ++i)
    	{
    		matrix[i] =(double*)calloc(matrixSize,sizeof(double));

    		for (j = 0; j < matrixSize; ++j)
    		{
    			matrix[i][j]=tempMatrix[k];	
                fprintf(out,"%0.f,",matrix[i][j]);
    			k++;
    		}
            fprintf(out,";");
    	}
        fprintf(out, "]\n");
        /*orjinal determinantı hesapla */
        Orjinaldet = Determinant(matrix,matrixSize);
        /*-----------   Matrixi 4 e bolme islemi    --------------------------------*/
        n = matrixSize;
        /*Result1 icin kullanılacak olan */
        R1m1 =(double**)calloc(n,sizeof(double));
        R1m2 =(double**)calloc(n,sizeof(double));
        R1m3 =(double**)calloc(n,sizeof(double));
        R1m4 =(double**)calloc(n,sizeof(double));
        /*Result2 icin kullanılacak olan*
        R2m1 =(double**)calloc(n,sizeof(double));
        R2m2 =(double**)calloc(n,sizeof(double));
        R2m3 =(double**)calloc(n,sizeof(double));
        R2m4 =(double**)calloc(n,sizeof(double));*/
        for(i = 0; i < n; i++)
        {
           /*----------------------------------------*/
            R1m1[i] =(double*)calloc(n,sizeof(double));
            R1m2[i] =(double*)calloc(n,sizeof(double));
            R1m3[i] =(double*)calloc(n,sizeof(double));
            R1m4[i] =(double*)calloc(n,sizeof(double));
            /*----------------------------------------*
            R2m1[i] =(double*)calloc(n,sizeof(double));
            R2m2[i] =(double*)calloc(n,sizeof(double));
            R2m3[i] =(double*)calloc(n,sizeof(double));
            R2m4[i] =(double*)calloc(n,sizeof(double));*/
            for(j = 0; j <n; j++)
            {
                if(i < n/2 && j < n/2)
                {
                   R1m1[i][j] = matrix[i][j];
                   /*R2m1[i][j] = matrix[i][j];*/
                }
                if(i >= n/2 && j < n/2)
                {
                    R1m2[i%(n/2)][j] = matrix[i][j];
                    /*R2m2[i%(n/2)][j] = matrix[i][j];*/
                }  
                if(i < n/2 && j >=2)
                {
                    R1m3[i][j%(n/2)] = matrix[i][j];
                    /*R2m3[i][j%(n/2)] = matrix[i][j];*/
                }
                if(n >= n/2 && j >= n/2)
                {
                    R1m4[i%(n/2)][j%(n/2)] = matrix[i][j];
                    /*R2m4[i%(n/2)][j%(n/2)] = matrix[i][j];*/
                }  
            }
        }
   		/*-----------------------------------------------------------------------*/

    	/*Show result fifos*/
    	while (((showfifo = open(showFifoName, O_WRONLY)) == -1) && (errno == EINTR)) ; 
    	if (showfifo == -1) {
    	  fprintf(stderr, "failed to open named pipe %s\n",showFifoName);
    	  return 1; 
    	} 
    	/* clientpid show resul'ta gonderme islemi*/
    	r_write(showfifo,pid,sizeof(int));
		close(showfifo);
		
		forkR1= fork();
		if(forkR1==0)
		{
            
            gettimeofday(&startR1, NULL);
            /*Parcalarin tek tek tersini alma islemi */
            R1m1 = Inverse(R1m1,n/2);
            R1m2 = Inverse(R1m2,n/2);
            R1m3 = Inverse(R1m3,n/2);
            R1m4 = Inverse(R1m4,n/2);
            
            /*--------------------------------------*/
            /*4 matristen tek bir matris yapma islemi */
            R1matrix = CompoundMatrix(R1m1,R1m2,R1m3,R1m4,matrixSize);
            fprintf(out, "[");
            for (i = 0; i < matrixSize; ++i)
            {
                for (j = 0; j < matrixSize; ++j)
                {
                    fprintf(out,"%0.f,",R1matrix[i][j]);
                }
                fprintf(out,";");
            }
            fprintf(out, "]\n");

		    /*Result1'i gondermek icin ozel fifo acıldı */
			Orjinaldet -= Determinant(R1matrix,matrixSize);

            gettimeofday(&stopR1, NULL);
            while (((showFifoData1 = open(showFifoDataName1, O_WRONLY)) == -1) && (errno == EINTR)) ; 
		   	if (showFifoData1 == -1) {
		   	  fprintf(stderr, "failed to open named pipe %s\n",showFifoDataName1);
		   	  return 1; 
		   	}
		   	/* result1 showResult'a gonderme islemi */
		   	result1[0]=Orjinaldet;
		   	r_write(showFifoData1,result1,sizeof(double));
		   	close(showFifoData1);
            /*Time hesabı yapma */
            elapsedTimeR1[0] =  (stopR1.tv_sec - startR1.tv_sec) * 1000.0;   
            elapsedTimeR1[0] += (stopR1.tv_usec - startR1.tv_usec) / 1000.0; 
            /*Time elapse yollama islemi */
            printf("TIME -> %.4f\n",elapsedTimeR1[0]);
            
            while (((showFifoData2 = open(showFifoDataName2, O_WRONLY)) == -1) && (errno == EINTR)) ; 
            if (showFifoData2 == -1) {
              fprintf(stderr, "failed to open named pipe %s\n",showFifoDataName2);
              return 1; 
            } 
            /* result2 showResult'a gonderme islemi */
            result2[0]=19.71;
            r_write(showFifoData2,elapsedTimeR1,sizeof(double));
    
            close(showFifoData2);    


            for (i = 0; i < matrixSize; ++i) free(R1matrix[i]);
            free(R1matrix);
		   	exit(0);
		}
        wait(NULL);

        /*Result2 Hesaplamaları yapılamadı */
        /*
		forkR2 =fork();
		if(forkR2==0)
		{   
            /*Result2'yi gondermek icin ozel fifo acıldı *
            printf("TIME -> %.4f\n",elapsedTimeR1[0]);
			while (((showFifoData2 = open(showFifoDataName2, O_WRONLY)) == -1) && (errno == EINTR)) ; 
		 	if (showFifoData2 == -1) {
		 	  fprintf(stderr, "failed to open named pipe %s\n",showFifoDataName2);
		 	  return 1; 
		 	} 
		    /* result2 showResult'a gonderme islemi *
		 	result2[0]=19.71;
		 	r_write(showFifoData2,elapsedTimeR1,sizeof(double));
	
		 	close(showFifoData2);
		 	exit(0);
		}
  	    wait(NULL);


        /*-----------------------------------------------------------------------*/
  	    /*Serverden request */
  	    while (((matrixFifo = open(matrisFifoName, O_WRONLY)) == -1) && (errno == EINTR)) ; 
  	    if (matrixFifo == -1) {
  	      fprintf(stderr, "failed to open named pipe %s\n",matrisFifoName);
  	      return 1; 
  	    }
  	    r_write(matrixFifo,pid,sizeof(int));
  	    close(matrixFifo);

  	    /*Free memory */
  	    for(i=0; i < matrixSize; i++)
        {
            free(matrix[i]);
            free(R1m1[i]);
            free(R1m2[i]);
            free(R1m3[i]);
            free(R1m4[i]);
            /*-------------*
            free(R2m1[i]);
            free(R2m2[i]);
            free(R2m3[i]);
            free(R2m4[i]);*/  
        }
        free(matrix);
        free(R1m1);
        free(R1m2);
        free(R1m3);
        free(R1m4);         
        /*-------------*
        free(R2m1);
        free(R2m2);
        free(R2m3);
        free(R2m4);*/

        printf("Request-> %d\n",pid[0]);
	}
	/*
	close(matrixFifo);
	unlink(matrixFifo);
	*/return 0;
}

void SignalHandler(int signo)
{
    FILE *out;
    char buf[BUFSIZE];
    sprintf(buf,"%d",mypid);
    unlink(buf);
    sprintf(buf,"%ds1",mypid);
    unlink(buf);
    sprintf(buf,"%ds2",mypid);
    unlink(buf);
    sprintf(buf,"log/%d.log",mypid);
    out = fopen(buf,"a");
    fprintf(out,"\nCTRL+C");
    close(out);
    kill(mypid, SIGINT);
    /*ve saire**/
    exit(signo);
}



double** Inverse(double** matrix, int n)
{
    int i, j, k;
    double ratio,a;
    for(i = 0; i < n; i++){
        for(j = n; j < 2*n; j++){
            if(i==(j-n))
                matrix[i][j] = 1.0;
            else
                matrix[i][j] = 0.0;
        }
    }
    
    for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
            if(i!=j){
                ratio = matrix[j][i]/matrix[i][i];
                for(k = 0; k < 2*n; k++){
                    matrix[j][k] -= ratio * matrix[i][k];
                }
            }
        }
    }

    for(i = 0; i < n; i++){
        a = matrix[i][i];
        for(j = 0; j < 2*n; j++){
            matrix[i][j] /= a;
        }
    }

    for(i = 0; i < n; i++){
        
        for(j = n; j < 2*n; j++){
            matrix[i][j%n] = matrix[i][j];    
        }
    }
    return matrix;
}

double **CompoundMatrix(double** m1,double** m2,double** m3,double** m4, int n )
{
    double **matrix;
    int i,j;
    matrix = (double**)calloc(n,sizeof(double));
    for (i = 0; i < n; ++i)
    {
        matrix[i] = (double*)calloc(n,sizeof(double));
        for (j = 0; j < n; ++j)
        {
            if(i < n/2 && j < n/2)
            {
                matrix[i][j] = m1[i][j];
            }
            if(i >= n/2 && j < n/2)
            {
                matrix[i][j] = m2[i%(n/2)][j];
            }
            if (i < n/2 && j >=n/2)
            {
                matrix[i][j] = m3[i][j%(n/2)];
            }
            if (n >= n/2 && j >= n/2)
            {
                matrix[i][j] = m4[i%(n/2)][j%(n/2)];
            }
        }
    }
    return matrix;
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
        bufp += byteswritten, bytestowrite -= byteswritten) 
    {
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
