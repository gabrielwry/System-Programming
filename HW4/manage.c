/* THIS CODE IS MY OWN WORK. IT WAS WRITTEN WITHOUT CONSULTING
 * A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Runye Wang */

#include "hw4.h"

int smid, mqid, cp_pid, cp_count, result_count;
segment *memory;

void mg_handler(int signum) {//handle termination

	for (int i = 0; i < MAX_PROCESS; i++) {
		cp_pid = memory->process[i].pid;
		//printf("%d\n",cp_pid);
		if(cp_pid == 0){
			break;//no more running cp processes
		}
		if (kill(cp_pid, SIGINT) != 0) {
			printf("Kill process %d failed. Error: %s\n",cp_pid, strerror(errno));
			exit(1);
		}
	}
	sleep(5);
	if (shmdt(memory) == -1) {
		printf("Can not detach shared memory %d. Error: %s\n",smid, strerror(errno));
		exit(1);
	}
	if (shmctl(smid, IPC_RMID, 0) == -1) {
		printf("Can not destroy shared memory %d. Error: %s\n",smid,strerror(errno));
		exit(1);
	}
	if (msgctl(mqid, IPC_RMID, NULL)==-1) {
		printf("Can not destroy message queue %d. Error: %s\n",mqid,strerror(errno));
		exit(1);
	}
	exit(0);
}

int main(int argc, char *argv[]) {
	/*=====Set up shared memory and message queue=====*/

	msg *message = malloc((sizeof(msg)));//reserve memory for message

	smid = shmget(SM_KEY, sizeof(segment), IPC_CREAT | IPC_EXCL | PERM);

	if (smid == -1) {
		printf("Failed to initate shared memory. Error: %s\n",strerror(errno));
		exit(1);
	}
	memory = shmat(smid, NULL, 0);

	if(memory == (void *) -1){
		printf("Failed to attach shared memory. Error: %s\n",strerror(errno));
		exit(1);
	}

	mqid = msgget(MQ_KEY, IPC_CREAT | IPC_EXCL | PERM);

	if (mqid == -1) {
		printf("Failed to initate message queue. Error: %s\n",strerror(errno));
		exit(1);
	}
	//printf("Message queue %d succeed \n",mqid );
	/*=======Overwrite INT QUIT HUNGUP signal action======*/
	struct sigaction action;
	action.sa_handler = mg_handler;
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGQUIT, &action, NULL);
	sigaction(SIGHUP, &action, NULL);

	while (1) {//keep recieving message from message queue
		msgrcv(mqid, message, sizeof(message->data), -3, 0);//only recieve initiate, kill and found perfect number message
		//printf("Message flag %d, data %d.\n",message->flag, message->data);
		if (message->flag == INIT_FLAG) {//initiate stats for compute process
			cp_pid = message->data;
			memory->process[cp_count].pid = cp_pid;
			memory->process[cp_count].tested = 0;
			memory->process[cp_count].found = 0;
			memory->process[cp_count].skipped =0;
			//send back index in message
			message->flag = INIT_SUCCESS_FLAG;
			message->data = cp_count;
			cp_count++;
			if(cp_count > MAX_PROCESS){
				printf("Error: Too many processes\n");
				exit(1);
			}
			if(msgsnd(mqid,message,sizeof(message->data),0) == -1){
				printf("Send back index for process %d failed. Error: %s\n", cp_pid,strerror(errno));
			}
			printf("Initating stats for process %d succeed\n",cp_pid);
		} 
		else if (message->flag == FOUND_FLAG) {
			memory->result[result_count] = message->data;
			result_count++;
			if(result_count > MAX_PERFECT){
				printf("Warning: This should never get printed, otherwise you win a Nobel Prize\n");//20 perfect number is too muchhhhh
			}
		} 
		else if (message->flag == KILL_FLAG) {
			cp_pid = memory->process[message->data].pid;//kill target process with index
			kill(cp_pid,SIGINT);
		}
	}
}
