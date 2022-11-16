#include <stdio.h>
#include "prioque.h"
#include "process.h"

int process_compare(const void *e1, const void *e2) {
	Process *p1 = (Process *)e1;
	Process *p2 = (Process *)e2;
	if (p1->pid == p2->pid) {
		return 1;
	} else {
		return 0;
	}
}

int execute_process(Process *process)
{
    process->usage++;
    //why it is always two, because pid is always 0
    if(process->pid != 0)
    {

        process->CPU_time--;
        if(process->CPU_time == 0)
        {

            if(process->IOtime > 0)
            {

                process->CPU_time = process->savedTime;
                return DOIO;
                //do IO
            }
            else
            {
                return FINISHED;
                //finished
            }
        }
        else
        {
            return NOTFINISHED;
            //not finished
        }
    }
    else
    {
        return FINISHED;
        //finished
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
        return FINISHED;
    }
    else
    {
     return NOTFINISHED;
    }
}

