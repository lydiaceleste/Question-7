#define DOIO 0
#define NOTFINISHED 1
#define FINISHED 2


typedef struct Process
{
    int pid;
    int arrival_time;
    int CPU_time;
    int savedTime;
    int repeat;
    int IOtime;
    int savedIOTime;
    int usage; //report usage
    int priority;
    int promote;
    int demote;
    int quantum;
    Queue Behaviors;

} Process;

typedef struct ProcessBehavior
{
    unsigned long CPU_burst;
    unsigned long IO_burst;
    int repeat;

} ProcessBehavior;

int process_compare(const void *e1, const void *e2);
int execute_process(Process *process);
int execute_IO(Process *process);
void init_process(Process *process);

