#include <stdio.h>
#include <stdlib.h>
#include "prioque.h"


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

// Queues
Queue ArrivalQueue, HighQueue, MediumQueue, LowQueue, IOQueue;

//variables
int quantum = 0; //CPU given to processes in individual queues
int result = 2; // 0 for doing IO, 1 for not finished, 2 for finished
int CPU; //model CPU clock


//special processes
Process nullProcess; //null process
Process *pointer; // points to null or the highest priority
Process preReadyProcess;
Process removedProcess;

void read_process_descriptions(void)
{
    Process process;
    ProcessBehavior b;
    int pid = 0, first = 1;
    unsigned long arrival;

    init_process(&process);
    arrival = 0;
    while (scanf("%lu", &arrival) != EOF)
    {
        scanf("%d %lu %lu %d", &pid, &b.CPU_burst, &b.IO_burst, &b.repeat);

        if (!first && process.pid != pid)
        {
            add_to_queue(&ArrivalQueue, &process, process.arrival_time);
            init_process(&pointer);
        }
        process.pid = pid;
        process.arrival_time = arrival;
        first = 0;
        add_to_queue(&process.Behaviors, &b, 1);
    }
    add_to_queue(&ArrivalQueue, &process, process.arrival_time);
}

void init_all_queues()
{
    init_queue(&ArrivalQueue, sizeof(Process), FALSE, process_compare, FALSE);
    init_queue(&HighQueue, sizeof(Process), FALSE, process_compare, FALSE);
    init_queue(&MediumQueue, sizeof(Process), FALSE, process_compare, FALSE);
    init_queue(&LowQueue, sizeof(Process), FALSE, process_compare, FALSE);
    init_queue(&IOQueue, sizeof(Process), FALSE, process_compare, TRUE);
}

void final_report()
{
    printf("Scheduler shutdown at time %d.\n", CPU - 1);
    printf("Total CPU usage for all processes scheduled:\n");
   // printf("Process <<null>>:\t%d time units.\n", NullProcess.CPU_Usage - 1);
   // puts(report);
}

//intialize a process with its characteristics
void init_process(Process *process)
{
    process->usage;
    process->pid;
    process->priority;
    process->demote;
    process->promote;
    process->quantum;
    init_queue(&(process->Behaviors), sizeof(ProcessBehavior), TRUE, NULL, TRUE);

}

void schedule_queues(Process *process)
{
    if(process->priority == 1)
    {
        add_to_queue(&HighQueue, process, process->quantum);
    }
    else if (process->priority == 2)
    {
        add_to_queue(&MediumQueue, process, process->quantum);
    }
    else if (process->priority == 3)
    {
        add_to_queue(&LowQueue, process, process->quantum);
    }
}

void preemption()
{
    Process current;
    ProcessBehavior behavior;
    init_process(&current);
    rewind_queue(&ArrivalQueue);
    peek_at_current(&ArrivalQueue, &current);

    if(CPU == current.arrival_time)
    {
        remove_from_front(&(current.Behaviors), &behavior);
        current.CPU_time = behavior.CPU_burst;
        current.savedTime = behavior.CPU_burst;
        current.IOtime = behavior.IO_burst;
        current.savedIOTime = behavior.IO_burst;
        current.repeat = behavior.repeat;
        schedule_queues(&current);
        printf("CREATE: Process %d entered the ready queue at time %d\n", current.pid, CPU);
    }
}
//promoting based on priority_algorithm()
void promote(Process *process)
{
    process->priority--;
    if(process->priority == 2)
    {
        process->demote = 2;
        process->promote = 2;
        process->quantum = 30;
    }
    else
    {
        process->demote = 1;
        process->quantum = 30;
    }
}

//demoting based on priority_algorithm()
void demote(Process *process)
{
    process->priority++;
    if(process->priority == 2)
    {
        process->demote = 2;
        process->promote = 2;
        process->quantum = 30;
    }
    else
    {
        process->demote = 1;
        process->quantum = 100;
    }
}

void priority_algorithm()
{
    //option 1, quantum at 0 but process is not complete
    if(quantum == 0)
    {
        if(result == 1)
        {
            if(pointer->priority < 3)
            {
                pointer->demote--;
                if(pointer->demote == 0)
                {
                    demote(pointer);
                }
            }
            else
            {
                pointer->quantum = 100;
            }
            schedule_queues(pointer);
            printf("QUEUED: Process %d queued at level %d at time %d.\n", pointer->pid, pointer->priority, CPU);
            pointer = &nullProcess;
        }
    }
    //case 2
    if(result == 0)
    {
        if(pointer->priority > 1)
        {
            pointer->promote--;
            if(pointer->promote == 0)
            {
                promote(pointer);
            }
        }
        else
        {
            pointer->quantum = 10;
        }
        printf("I/O: Process %d blocked for I/O at time %d\n", pointer->pid, CPU);
        add_to_queue(&IOQueue, pointer, 1);
        pointer = &nullProcess;
    }
    //case 3, finished
    else if(result == 2)
    {
        if(process_compare(&nullProcess, pointer))
        {
            //process report
            char process_report[50];
            printf("FINISHED: Process %d finished at time %d.\n", pointer->pid, CPU);
            printf(process_report, "Process %d:\t\t%d time units.\n", pointer->pid, pointer->usage);
            pointer = &nullProcess;
        }
    }
    //when nullP is null
    if(!compare_processes(&nullProcess, pointer))
    {
        if(!(empty_queue(&HighQueue) && empty_queue(&LowQueue) && empty_queue(&MediumQueue)))
        {
            if(!empty_queue(&HighQueue))
            {
                remove_from_front(&HighQueue, &preReadyProcess);
            }
            else if (!empty_queue(&MediumQueue))
            {
                remove_from_front(&MediumQueue, &preReadyProcess);
            }
            else if (!empty_queue(&LowQueue))
            {
                remove_from_front(&LowQueue, &preReadyProcess);
            }
            removedProcess = preReadyProcess;
            pointer = &removedProcess;
            quantum= pointer->quantum;
            printf("RUN: Process %d started execution from level %d at time %d; wants to execute for %lu ticks.\n", pointer->pid, pointer->priority, CPU, pointer->CPU_time);
        }
    }
    //looking for higher priority process
    else
    {
        if (!empty_queue(&HighQueue) || !empty_queue(&MediumQueue))
        {
            if(!empty_queue(&HighQueue) && pointer->priority > 1)
            {
                remove_from_front(&HighQueue, &preReadyProcess);
            }
            else if(!empty_queue(&MediumQueue) && pointer->priority > 2)
            {
                remove_from_front(&MediumQueue, &preReadyProcess);
            }
            if(pointer->priority > preReadyProcess.priority)
            {            
                printf("QUEUED: Process %d queued at level %d at time %d.\n" , pointer->pid, pointer->priority, CPU);
                pointer->quantum = quantum;
                schedule_queues(pointer);
                removedProcess = preReadyProcess;
                pointer = &removedProcess;
                quantum = pointer->quantum;
                printf("RUN: Process %d started execution from level %d at time %d; wants to execute for %lu ticks.\n", pointer->pid, pointer->priority, CPU, pointer->CPU_time);

         }
      }
    }
}

//do any more processes exist?
int processes_exist()
{
    return (!(empty_queue(&HighQueue) && empty_queue(&LowQueue) && empty_queue(&MediumQueue)) || !empty_queue(&IOQueue) || !empty_queue(&ArrivalQueue) || process_compare(&nullProcess, &pointer));
}

void input_output()
{
    if (!empty_queue(&IOQueue))
    {
        Process IO;
        init_process(&IO);
        for(int n = 0; n < IOQueue.queuelength; n++)
        {
            remove_from_front(&IOQueue, &IO);
            int result = execute_IO(&IO);
            if (result != 2)
            {
                add_to_queue(&IOQueue, &IO, 1);
            }
            else
            {
                schedule_queues(&IO);
            }
        }
    }
}

int execute_process(Process *process)
{
    process->usage++;
    if(process->pid != 0)
    {
        process->CPU_time--;
        if(process->CPU_time == 0)
        {
            if(process->IOtime > 0)
            {
                process->CPU_time = process->savedTime;
                return 0;
            }
            else
            {
                return 2;
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 2;
    }
}

int execute_IO(Process *process)
{
    process->IOtime--;
    if(process->IOtime == 0)
    {
        process->repeat--;

        if(process->repeat > 0)
        {
            process->IOtime = process->savedIOTime;
        }
        else
        {
            if(!empty_queue(&(process->Behaviors)))
            {
                ProcessBehavior behavior;
                remove_from_front(&(process->Behaviors), &behavior);
                process->CPU_time = behavior.CPU_burst;
                process->savedTime = process->CPU_time;
                process->IOtime = behavior.IO_burst;
                process->savedIOTime = process->IOtime;
                process->repeat = behavior.repeat;
            }
        }
        return 2;
    }
    else
    {
        return 1;
    }
}

int process_compare(const void *e1, const void *e2)
{
    Process *p1 = (Process *)e1;
    Process *p2 = (Process *)e2;
    if(p1->pid != p2->pid)
    {
        return 1;
    }
    else
    {
        return 0;
    }

}

int main(int argc, char *argv[])
{
    init_all_queues();
    init_process(&nullProcess);
    pointer = &nullProcess;
    init_process(&preReadyProcess);
    init_process(&removedProcess);
    read_process_descriptions();

    while (processes_exist())
    {
        CPU++;
        preemption();
        priority_algorithm();
        input_output();
    }
    CPU++;
    final_report();
    return 0;
}