/* THIS CODE IS MY OWN WORK. IT WAS WRITTEN WITHOUT CONSULTING
 * A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Runye Wang*/



#include "hw4.h"

int smid, mqid, pid, cp_index;
int cand, sum;
bool finished;
segment *memory;

void cp_handler(int signum) {//termination handling

	memory->process[cp_index].pid = 0;
	memory->process[cp_index].found = 0;
	memory->process[cp_index].tested = 0;
	memory->process[cp_index].skipped = 0;

	exit(0);
}
int whichSeg(int cand){
	return cand / (INT_PER_SEG * 8 * sizeof(int));
}

int whichInt(int cand) {
	return (cand % (INT_PER_SEG * 8 * sizeof(int))) / (8 * sizeof(int));
}

int whichBit(int cand) {
	return cand - whichSeg(cand) * INT_PER_SEG * 8 * sizeof(int) - whichInt(cand) * 32; //this is ugly
}

int test(int cand) {//test perfect number
	sum = 0;
	for (int i = 1; i < cand; i++) {
		if (cand % i == 0) {
			sum += i;
		}
	}
	if (sum == cand) {
		return 1;
	} else {
		return 0;
	}
}

int tested(int cand){
	return (memory->bitMap[whichSeg(cand)][whichInt(cand)] & (1 << whichBit(cand)));
}


int main(int argc, char *argv[]) {
	//("Test 131073 seg: %d, int %d, bit %d\n",whichSeg(131073),whichInt(131073),whichBit(131073));
	/*=========Same rountine for set up phase===========*/
	msg *message = malloc((sizeof(msg)));
	if (argc != 2) {
		printf("usage: %s start number\n",argv[0]);
		exit(0);
	}

	int start = atoi(argv[1]);
	if (start > RANGE || start < 2) {
		printf("Out of range. Range is 2~33554432\n");
		exit(0);
	}
	smid = shmget(SM_KEY, sizeof(segment), 0);
	if (smid == -1 ) {
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
	//Send message to manager for initiation
	pid = getpid();
	message->flag = INIT_FLAG;
	message->data = pid;
	if (msgsnd(mqid, message, sizeof(message->data), 0) == -1) {
		printf("Can not initiate stats for process %d. Error: %s\n",pid,strerror(errno));
		exit(1);
	}
	if(msgrcv(mqid, message, sizeof(message->data), INIT_SUCCESS_FLAG, 0) == -1){
		printf("Initiation message recieve failed.\n");
		exit(1);
	}
	cp_index = message->data;
	//printf("Cp index is %d\n",cp_index );
	//Same signal overwrite rountine
	struct sigaction action;
	action.sa_handler = cp_handler;
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGQUIT, &action, NULL);
	sigaction(SIGHUP, &action, NULL);

	cand = start;
	//printf("Set up success\n");
	while (1 && (!finished)) {
		if (cand > RANGE) {
			cand = start;
			finished = true;
			cp_handler(SIGQUIT);//wrap up and get back to start position
		}

		if ( !tested(cand)) {
			memory->bitMap[whichSeg(cand)][whichInt(cand)] |= (1 << whichBit(cand));//mark as tested
			memory->process[cp_index].tested++;
			if (test(cand)) {
				message->flag = FOUND_FLAG;
				message->data = cand;
				if (msgsnd(mqid, message, sizeof(message->data), 0) == -1) {
					printf("Perfect number send failed. Error: %s\n",strerror(errno));
					exit(1);
				}
				memory->process[cp_index].found++;
			} 
		} 
		else {
			//printf("Skipping %d\n",cand );
			memory->process[cp_index].skipped++;
		}
		cand++;
	}
}
