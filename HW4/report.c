/* THIS CODE IS MY OWN WORK. IT WAS WRITTEN WITHOUT CONSULTING
 * A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Runye Wang */



#include "hw4.h"

int smid, mqid;
int total_tested, total_found, total_skipped;
int pid, p_tested, p_found, p_skipped;
segment *memory;

int main(int argc, char *argv[]) {
	msg *message = malloc((sizeof(msg)));
	smid = shmget(SM_KEY, sizeof(segment), 0);
	if (smid == -1) {
		printf("Failed to get shared memory. Error: %s\n",strerror(errno));
		exit(1);
	}
	memory = shmat(smid, NULL, 0);
	if (memory == (void *) -1) {
		printf("Failed to attach shared memory. Error: %s\n",strerror(errno));
		exit(1);	
	}
	mqid = msgget(MQ_KEY, 0);
	if (mqid == -1) {
		printf("Failed to get message queue. Error: %s\n",strerror(errno));
		exit(1);
	}

	if (argc >= 2) {
		if (strcmp(argv[1], "-k") == 0) {
			for(int i=0;i<MAX_PROCESS;i++){
				message->flag = KILL_FLAG;
				message->data = i;
				if(memory->process[i].pid == 0){//no more running processes
					break;
				}
				if (msgsnd(mqid, message, sizeof(message->data), 0) != 0) {
					printf("Kill signal failed Error: %s\n",strerror(errno));
					exit(1);
				}
			}
		} 
		else {
			printf("usage: %s [-k]\n",argv[0]);
			exit(0);
		}
	}

	printf("Perfect numbers found: ");
	for (int i = 0; i < MAX_PERFECT; i++) {
		if(memory->result[i] == 0){//no more perfect number
			break;
		}
		printf("%d ",memory->result[i]);
	}
	printf("\n");

	for (int i = 0; i < MAX_PROCESS; i++) {
		pid = memory->process[i].pid;
		if(pid == 0){//no more running process
			break;
		}
		p_tested = memory->process[i].tested;
		p_found = memory->process[i].found;
		p_skipped = memory->process[i].skipped;
		printf("Running process %d tested : %d numbers, found : %d perfect numbers,and skipped %d numbers.\n", pid,p_tested,p_found,p_skipped);
		total_tested += p_tested;
		total_found += p_found;
		total_skipped += p_skipped;
	}

	printf("For currently running processes, total number tested:  %d, total found: %d, total skipped : %d\n", total_tested, total_found, total_skipped);
}
