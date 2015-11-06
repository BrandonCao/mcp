/*****

Brandon Cao


*****/
#include <stdio.h>
#include <wordexp.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <proc/readproc.h>


#define MAXLEN 256

/***************************************
 * 
 * Signal handler function for SIGALRM
 *
 * 
 *****************************************/ 

typedef void (*sighandler_t)(int sig); //Stackoverflow 

volatile int alarmHandler;
void signalHandler(int t ) {
	alarmHandler = 1;

}


int main(int argc, char *argv[]){

	FILE *file = fopen(argv[1],"r");	
	int i, len, sig,status;
	int numPrograms = 0;
	int runningProcess = 1; 
	wordexp_t wordStruct;
	wordexp_t wordArray[MAXLEN];
	char buffer[MAXLEN][MAXLEN];
	char line[MAXLEN];
	sigset_t sigSet;
	pid_t pid[numPrograms];
	pid_t tempChild;
	pid_t procPID[numPrograms];
	signal(SIGALRM,signalHandler);
	sigemptyset(&sigSet);
	sigaddset(&sigSet, SIGUSR1);
	sigprocmask(SIG_BLOCK,&sigSet,NULL);
	PROCTAB* proc;
	proc_t* procInfo;
	proc_t info;

	

	if( file != NULL){
		/***PART 1***/
		char* get;
		while((get = fgets(line, sizeof(line),file)) != NULL) {
			len = strlen(line);
			if(line[len-1] == '\n'){
				line[len-1] = '\0';
			}
			memset(&wordStruct, 0, sizeof(wordStruct));
			strcpy(buffer[numPrograms], line);
			wordexp(buffer[numPrograms],&wordStruct,0);
			wordArray[numPrograms] = wordStruct;
			numPrograms++;
	
		}
			
		for(i = 0; i < numPrograms; ++i){
			/***fork child process***/
			tempChild = fork();
			if(tempChild < 0){
				perror("failed to fork\n");
				exit(1);
			}
			if(tempChild ==0){
				/***part 2***/
				sigwait(&sigSet,&sig); 
				if(execvp(wordArray[i].we_wordv[0], wordArray[i].we_wordv) == -1){
					perror("Execution failed\n");
				}
			}
			else if ( tempChild > 0) {
				pid[i] = tempChild;
				wordfree(&(wordArray[i]));
			}			
		}
				
		/***Wake up child process***/
		for(i = 0; i < numPrograms; ++i){
			kill(pid[i],SIGUSR1);
			printf("Process %d signaled SIGUSR1\n",pid[i]);
		
		}
		
		/**Stop child process**/
		for(i = 0; i < numPrograms; ++i){
			kill(pid[i],SIGSTOP);
			printf("Process %d signaled SIGSTOP\n",pid[i]);		
		}
		
/***************************************
 * 
 * This while loop was used in part 2 of the mcp project
 * but removed.
 
  removed from part 2
		 * for(i = 0; i < numPrograms; ++i){
		 * 	kill(pid[i],SIGCONT);
		 * printf("Child %d is continuing\n",i);
		 * waitpid(-1,&pid[i],0);
		 *} 

 * 
 *****************************************/ 

	
		/**************
		 * Got idea of this while loop from office hours
		 * 
		 * 
		 * while flag{
		 * 
		 * 	flag = false;
		 * for id in pids:
		 * 	if running ID
		 *	flag == true
		 * 	//ran process
		 * 
		 * }
		 * 
		 ***************/
		 
		 /**flag **/
		while(runningProcess == 1 ){
			runningProcess = 0;
			
			for( i = 0; i < numPrograms; ++i){
				if(waitpid(pid[i], &status, WNOHANG) == 0){	
					
					runningProcess = 1;
					printf("Process %d resumed\n",pid[i]);
					printf("----------------------\n");
					kill(pid[i],SIGCONT);
					/**PART 4**/
					proc = openproc(PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS| PROC_PID,pid);
					memset(&info, 0, sizeof(info));
					printf("%20s:\t%s\t%5s\n", "COMMAND","PRIORITY","PPID(PID of parent)");
					printf("%20s\t%s\t%5s\n", "--------","--------","----");
					
					while((procInfo = readproc(proc,&info)) != NULL){
						printf("%20s:\t%lu\t\t%5i\n",
						info.cmd, info.priority,info.ppid);
					}
			
					closeproc(proc);
					
					/**PART 3 alarm handler**/
					alarm(1);
					
					/**Interupts the MCP**/
					while(alarmHandler == 0){
						;
					}
					alarmHandler = 0;
					kill(pid[i],SIGSTOP);
					
					printf("----------------------\n");
					printf("Stopping process %d \n",pid[i]);
					printf("----------------------\n");
					
					}
					
				}
		}
	}
	
	
	
	printf("\nChild processes complete\nProgram Complete\n");
	fclose(file);
						
}