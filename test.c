#include <stdio.h>					
#include <stdlib.h>					
#include <string.h>					
#include <limits.h>					
#include "prioque.h"

#define FINISHED 'F'
#define QUEUED 'Q'
#define IO 'I'
#define RUN 'R'


typedef struct _Process {
	int pid ;
	int arrival_time ;
	int priority;
	int b;		//demotion counter
	int g;		//promotion counter
	int totalTime;
	int q;
	Queue behaviors;
} Process;

typedef struct _ProcessBehavior {
	int cpuburst;
	int cpuburstLeft;
	int ioburst;
	int repeat;
	int ticksRun;
	int ticksIO;
} ProcessBehavior;


void read_process_descriptions(void);
void queue_new_arrivals();
void execute_highest_priority_process();
void init_all_queues();
void init_process(Process *p);
void final_report();
void do_IO();
int processes_exist();
void check_promotion();
void print_info(Process, char);

Process IdleProcess;
ProcessBehavior dummyBehavior;
Process *processHolder = NULL;

Queue ArrivalQ;
Queue PrioQ;
Queue IOQ;
Queue FinishedQ;

int Clock = 0;
int running = 0;

int processCmp(const void *e1, const void *e2) {
	Process *p1 = (Process *)e1;
	Process *p2 = (Process *)e2;
	if (p1->pid == p2->pid) {
		return 1;
	} else {
		return 0;
	}
}

int main(int argc, char *argv[]) {
	
	init_all_queues();
	init_process(&IdleProcess);
	read_process_descriptions();
	while (processes_exist()) {
		Clock++;
		queue_new_arrivals();
		execute_highest_priority_process();
		do_IO();
	}

	Clock++;
	final_report();

	return 0;
}

//printf final results of the scheduler
void final_report() {
	Process temp;
	printf("Scheduler shutdown at time %d.\n", Clock);
	printf("Total CPU usage for all processes scheduled:\n");
	printf("Process <null>:\t%d time units.\n", IdleProcess.totalTime);
	while (! empty_queue(&FinishedQ)) {
		remove_from_front(&FinishedQ, &temp);
		printf("Process <%d>:\t%d time units.\n", temp.pid, temp.totalTime);
	}
	return;
}

//Counts I/O blocking and exits once finished
void do_IO() {
	Process processTemp;
	ProcessBehavior behTemp;

	//if the IOQ is empty, exit; otherwise perform IO tick
	if (!empty_queue(&IOQ)) {
		processTemp = *((Process*)pointer_to_current(&IOQ));
		behTemp = *((ProcessBehavior*)pointer_to_current(&processTemp.behaviors));
	} else {
		return;
	}

	//if IO is finished, decrement repeat and add process back to PrioQ
	if (behTemp.ticksIO == behTemp.ioburst) {
		behTemp.repeat--; 
		behTemp.ticksIO = 0;
		update_current(&processTemp.behaviors, &behTemp);
		update_current(&IOQ, &processTemp);
		delete_current(&IOQ);
		add_to_queue(&PrioQ, &processTemp, processTemp.priority);
		return;
	}

	//IO tick
	behTemp.ticksIO++;
	update_current(&processTemp.behaviors, &behTemp);
	update_current(&IOQ, &processTemp);
	return;
}

//Queues process to PrioQ once their arrival time is met
void queue_new_arrivals() {
	Process temp;
	if (! empty_queue(&ArrivalQ) && Clock == current_priority(&ArrivalQ)) {
		remove_from_front(&ArrivalQ, &temp);
		add_to_queue(&PrioQ, &temp, 1);
		printf("CREATE: Process %d entered the ready queue at time %d.\n", temp.pid, Clock);
	}
	return;
}

//returns 0 if all processes are done executing, otherwise return a non-zero integer
int processes_exist() {
	return queue_length(&ArrivalQ) + queue_length(&PrioQ) + queue_length(&IOQ);
}

//initializes base process's attributes and the ProcessBehavior queue
void init_process(Process *p) {
	init_queue(&p->behaviors, sizeof(ProcessBehavior), TRUE, processCmp, FALSE);
	p->b = 0;
	p->g = 0;
	p->q = 10;
	p->priority = 1;
	p->totalTime = 0;
	return;
}

//initializes all queues
void init_all_queues() {
	init_queue(&PrioQ, sizeof(Process), TRUE, processCmp, FALSE);
	init_queue(&ArrivalQ, sizeof(Process), TRUE, processCmp, FALSE);
	init_queue(&IOQ, sizeof(Process), TRUE, processCmp, FALSE);
	init_queue(&FinishedQ, sizeof(Process), TRUE, processCmp, FALSE);
	return;
}

//read all processes descriptions from standard input and populate
//the 'ArrivalQueue'.
void read_process_descriptions(void) {
	Process p;
	ProcessBehavior b;
	int pid = 0, first = 1;
	unsigned long arrival;

	init_process(&p);
	arrival = 0;
	while (scanf("%ld", &arrival)!= EOF) {
		scanf("%d %d %d %d",
			&pid,
			&b.cpuburst,
			&b.ioburst,
			&b.repeat);

		if (! first && p.pid != pid) {
			add_to_queue(&ArrivalQ, &p, p.arrival_time);
			init_process(&p);
		}
		p.pid = pid;
		p.arrival_time = arrival;
		first = 0;
		b.ticksRun = 0;
		b.ticksIO = 0;
		b.cpuburstLeft = b.cpuburst;
		add_to_queue(&p.behaviors, &b, 1);

	}
	add_to_queue(&ArrivalQ, &p, p.arrival_time);
	return;
}

//Executes the highest priority process for one ticke
//handles exit cases for io, queuing, and finishing
void execute_highest_priority_process() {
        ProcessBehavior behTemp;
        ProcessBehavior peekIOBeh;
        Process processTemp;
	Process peekIOProcess;

	//If no processes in PrioQ, exit; otherwise perform run tick
	if (empty_queue(&PrioQ)) {
		IdleProcess.totalTime++;
		return;
	} 
	
	//checks if a process is currently running	
	if (running) {
		processTemp = *processHolder;
		while (! processCmp(pointer_to_current(&PrioQ), &processTemp)) {
			next_element(&PrioQ);
		}
	} else {
 		processTemp = *((Process*)pointer_to_current(&PrioQ));
		processHolder = (Process*)pointer_to_current(&PrioQ);
		running = 1;
	}
	//if a process exits IO during a run
	if (empty_queue(&IOQ)) {
		peekIOBeh = dummyBehavior;
		peekIOBeh.ioburst = INT_MAX;
	} else {
		peekIOProcess = *((Process*)pointer_to_current(&IOQ));
		peekIOBeh = *((ProcessBehavior*)pointer_to_current(&peekIOProcess.behaviors));
	}
	behTemp = *((ProcessBehavior*)pointer_to_current(&processTemp.behaviors));

	if (behTemp.ticksRun == 0) {
		print_info(processTemp, RUN);
	}
        behTemp.ticksRun++;
	processTemp.totalTime++;


       	if (behTemp.ticksRun == behTemp.cpuburstLeft) {
		//a run has completed
		running = 0;
		behTemp.ticksRun = 0;
		processTemp.b = 0; 
		check_promotion(&processTemp);
		delete_current(&PrioQ);
		if (behTemp.repeat != 0) {
			//more repeats: add to IOQ
			behTemp.cpuburstLeft = behTemp.cpuburst;
			processTemp.g++;
			check_promotion(&processTemp);
			update_current(&processTemp.behaviors, &behTemp);
			add_to_queue(&IOQ, &processTemp, processTemp.priority);
			print_info(processTemp, IO);
		} else {
			//no more repeats: delete behavior
			delete_current(&processTemp.behaviors);
			if (empty_queue(&processTemp.behaviors)) {
				print_info(processTemp, FINISHED);
				add_to_queue(&FinishedQ, &processTemp, 1);
			} else {
				print_info(processTemp, IO);
				add_to_queue(&IOQ, &processTemp, processTemp.priority);
			}
		}
		return;
	} else if (behTemp.ticksRun == peekIOBeh.ioburst) {
		//a process is exiting IO
		running = 0;
		behTemp.cpuburstLeft -= peekIOBeh.ioburst;
		processTemp.g++;
		check_promotion(&processTemp);
		print_info(processTemp, QUEUED);
		behTemp.ticksRun = 0;
	} else if (behTemp.ticksRun == processTemp.q) {
		//running process maxed quantum
		running = 0;
		behTemp.cpuburstLeft -= processTemp.q;
		processTemp.b++;
		check_promotion(&processTemp);
		print_info(processTemp, QUEUED);
		behTemp.ticksRun = 0;
	}
	
	update_current(&processTemp.behaviors, &behTemp);
	update_current(&PrioQ, &processTemp);
	rewind_queue(&PrioQ);
	return;
}

//Checks promotion and demotion and applies it
void check_promotion(Process *p) {
	int prioB, prioG;

	if (p->priority == 1) {
		prioB = 1;
		prioG = INT_MAX;
	} else if (p->priority == 2) {
		prioB = 2;
		prioG = 2;
	} else {
		prioB = INT_MAX;
		prioG = 1;
	}
	
	if (p->b == prioB) {
		p->priority++;
		p->b = 0;
		p->g = 0;
	}
	if (p->g == prioG) {
		p->priority--;
		p->g = 0;
		p->b = 0;
	}
	
	switch (p->priority) {
		case 1:
			p->q = 10;
			break;
		case 2: 
			p->q = 30;
			break;
		case 3:
			p->q = 100;
			break;
	}
	return;
}


//prints useful information for the user
void print_info(Process p, char letter) {
	ProcessBehavior b;
	if (! empty_queue(&p.behaviors)) {
		b = *((ProcessBehavior*)pointer_to_current(&p.behaviors));
	}
	switch (letter) {
		case 'R':
			printf("RUN: Process %d started execution from level %d at time %d; wants to execute for %d ticks.\n", 
					p.pid, p.priority, Clock, b.cpuburstLeft);
			break;
		case 'Q':
			printf("QUEUED: Process %d queued at level %d at time %d.\n", p.pid, p.priority, Clock+1);
			break;
		case 'I':
			printf("I/O: Process %d blocked for I/O at time %d.\n", p.pid, Clock+1);
			break;
		case 'F':
			printf("FINISHED: Process %d finished at time %d.\n", p.pid, Clock+1);
			break;
	}	
	return;
}
