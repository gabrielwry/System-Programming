/*======This code is my own code without consulting other students or TA. Runye Wang*/


/*
	Stadin Usage: ./uniqify < [filename]
	Stdout output.
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int ps_fd[2];
int sm_fd[2];

void parse_process(int ps_fd[2]);
void sort_process(int ps_fd[2], int sm_fd[2]);
void merge_process(int sm_fd[2]);
void merge(int sm_fd[2]);

void parse_process(int ps_fd[2])
{
	int len=0;
	char buf[100];
	char word[36];
	char tmp[100];
	FILE *parse_sort_stream = fdopen(ps_fd[1],"w");
	if(parse_sort_stream == -1){
		fprintf(stderr, "Failed to create write end of parse-sort pipe\n");
		exit(EXIT_FAILURE);
	}
	while(fgets(buf,100,stdin)!=NULL){
		for(int i=0;i<99;i++){
			if(isalpha(buf[i])){
				tmp[len] = tolower(buf[i]);//to lower case if alpha
				len++;
			}
			else{
				tmp[len]='\n';//sort() is sorting LINES of text file!!!!!!!
				if(len >= 5){
					strncpy(word,tmp,36);//truncate anything longer than 35
					memset(tmp,0,100);
					fputs(word,parse_sort_stream);//put words to stream for sort
					memset(word,0,36);
				}
				len=0;
			}
		}
	}
	
	fclose(parse_sort_stream);
	//printf("Parsing finished\n");

}


void sort_process(int ps_fd[2],int sm_fd[2])
{
	int ps_pid = fork(); // fork sort process
	char buf[100];
	//printf("Is this ever called\n");
	switch(ps_pid){
		case -1:
			fprintf(stderr, "Failed to fork sort processes from parse\n");
			exit(EXIT_FAILURE);
			break;
		case 0: //child process -- redirect stdin and stdout for sort
			
			//printf("Sorting\n");
			
			dup2(ps_fd[0],STDIN_FILENO); // read from parse-sort pipe
			dup2(sm_fd[1],STDOUT_FILENO); // write to sort-merge pipe
			close(ps_fd[1]);
			close(sm_fd[0]); // make sure other ends are closed
			
			execlp("sort", "sort", (char *)NULL); 

			_exit(EXIT_FAILURE); //send signal to parent
			break;
		default: //parent process
			//printf("Parent process\n");
			close(ps_fd[0]);//close read end of parse-sort pipe
			close(sm_fd[1]);//close write end of sort-merge pipe
			break;

	}
}

void merge_process(int sm_fd[2]){
	int sm_pid = fork(); // fork merge process
	switch(sm_pid){
		case -1:
			fprintf(stderr, "Failed to fork merge process from sort process\n");
			exit(EXIT_FAILURE);
			break;
		case 0: //child process of merge
			merge(sm_fd);
			_exit(EXIT_FAILURE);
			break;
		default:
			//printf("waiting for sort\n");
			waitpid(sm_pid, NULL, 0);
			break;
	}
}
void merge(int sm_fd[2]){
	//printf("merging\n");
	char prev[36];
	char word[36];
	int freq = 1;
	FILE *sort_merge_stream = fdopen(sm_fd[0],"r");

	fgets(word,36,sort_merge_stream);
	strcpy(prev,word);//get first word

	while(fgets(word,36,sort_merge_stream)!=NULL){
   		if(strcmp(prev,word)==0){
   			freq++;
   		}
   		else{
   			for(int i=0;i<36;i++){
   				if(prev[i]=='\n'){
   					prev[i]='\0';//clear new line
   				}
   			}
   			printf("%s - %05d\n",prev,freq);//formating output
   			freq=1;
   			strcpy(prev,word);
   		}
	}
	fclose(sort_merge_stream);

}
int main(int argc, char *argv[])
{
	
	if(pipe(ps_fd) == -1){
		fprintf(stderr, "Failed to create parse-sort pipe\n");
		exit(EXIT_FAILURE);
	}
	if(pipe(sm_fd) == -1){
		fprintf(stderr, "Failed to create sort-merge pipe\n");
		exit(EXIT_FAILURE);
	}

	sort_process(ps_fd,sm_fd);
	parse_process(ps_fd);
	merge_process(sm_fd);

	close(ps_fd[1]);
	wait(NULL);//all parents wait for child process

	return 0;
}