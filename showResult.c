/**
* Burak Demirci
* 141044091
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>  
#include <sys/wait.h>
#include <string.h>

int mypid;
#define BUFSIZE 2048
#define FIFO_PERM  (S_IRUSR | S_IWUSR)
void SignalHandler(int signo);

ssize_t r_read(int fd, void *buf, size_t size) 
{
	/*Bu fonksiyon kitaptan alınmıştır*/
   ssize_t retval;
   while (retval = read(fd, buf, size), retval == -1 && errno == EINTR) ;
   return retval;
}

int main(int argc, char const *argv[])
{
	FILE *output;
	char fifoname[BUFSIZE]="showresult";
	int showfifo;
    int clientFifo,clientFifo1,clientFifo2;
    pid_t Forkpid1,Forkpid2,Forkpid;
	double result1[1],result2[1];
    double timeEl1[1],timeEl2[1];
    char requestFifoName1[BUFSIZE],requestFifoName2[BUFSIZE];
	char buf[BUFSIZE];
    int clientPid[1];
    mypid = (int)getpid();
    signal (SIGINT, SignalHandler); 
    output = fopen("log/showResult.log","a");
	if (mkfifo(fifoname,FIFO_PERM) == -1) 
	{
		/*Create showresult fifo fifo*/
        if (errno != EEXIST)
        {
    	   printf("failed to create named pipe %s\n", fifoname);
    	   return 1; 
  	    }
    }
   	
    while(1)
    {
        while (((showfifo = open(fifoname, O_RDONLY)) == -1) && (errno == EINTR)) ; 
        if (showfifo == -1) {
            printf("failed to open named pipe %s\n",fifoname);
            return 1; 
        }    
        r_read(showfifo,clientPid,sizeof(int)); 

        sprintf(requestFifoName1,"%ds1",clientPid[0]);
        sprintf(requestFifoName2,"%ds2",clientPid[0]);

        close(showfifo);
        Forkpid = fork();
        if( Forkpid == 0 )                   
        {
            while (((clientFifo1 = open(requestFifoName1, O_RDONLY)) == -1) && (errno == EINTR)) ; 
            if (clientFifo1 == -1) {
                printf("failed to open named pipe %s\n",requestFifoName1);
                return 1; 
            }
            r_read(clientFifo1,result1,sizeof(double));  
            close(clientFifo1);
            /*Read time elapsed for Result1 */ 
            
            while (((clientFifo2 = open(requestFifoName2, O_RDONLY)) == -1) && (errno == EINTR)) ; 
            if (clientFifo1 == -1) {
                printf("failed to open named pipe %s\n",requestFifoName2);
                return 1; 
            }
            r_read(clientFifo2,timeEl1,sizeof(double));
            close(clientFifo2);         
            printf("PID: %d Result1: %f \n",clientPid[0],result1[0]);
            fprintf(output,"Pid: %d  Result1: %f  TimeElapse1:%f\n",clientPid[0],result1[0],timeEl1[0]); 
            exit(0);
        }
        wait(0);    
    }
	return 0;
}
void SignalHandler(int signo)
{
    FILE *out;
    char buf[BUFSIZE];
    unlink("showresult");
    sprintf(buf,"log/showResult.log",mypid);
    out = fopen(buf,"a");
    fprintf(out,"\nCTRL+C");
    close(out);
    kill(mypid, SIGINT);
    /*ve saire**/
    exit(signo);
}
