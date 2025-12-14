#include <stdio.h>
#include <stdlib.h>

#include "../includes/process.h"
#include "../includes/simulation.h"
#include "../includes/utils.h"


#define AGING_QUANTUM 2

#define MAX_LEVELS 10

void schedule_mlq_aging(Process processes[], int num_processes, int quantum_unused)
{
    (void)quantum_unused;  

    ProcessQueue queues[MAX_LEVELS + 1];
    int time = 0;
    int finished = 0;

    for (int i = 0; i <= MAX_LEVELS; i++) {
        init_queue(&queues[i]);
    }

    for (int i = 0; i < num_processes; i++) {
        Process *p = &processes[i];

        p->remaining_time   = p->burst_time;
        p->dynamic_priority = p->priority;      
        p->quantum_left     = AGING_QUANTUM;   

        p->start_time  = -1;
        p->finish_time = 0;

        p->num_slices  = 0;
        p->is_started  = 0;                    
    }


    while (finished < num_processes) {

        for (int i = 0; i < num_processes; i++) {
            if (processes[i].arrival_time == time) {
                int pr = processes[i].dynamic_priority;
                if (pr < 0) pr = 0;
                if (pr > MAX_LEVELS) pr = MAX_LEVELS;

                enqueue(&queues[pr], &processes[i]);
            }
        }

        int lvl = -1;
        for (int p = MAX_LEVELS; p >= 0; p--) {
            if (!is_empty(&queues[p])) {
                lvl = p;
                break;
            }
        }

        if (lvl == -1) {
            time++;
            continue;
        }

        Process *current = dequeue(&queues[lvl]);

        if (current->start_time == -1) {
            current->start_time = time;
        }

        current->remaining_time--;
        current->quantum_left--;

        int s = current->num_slices;
        current->slices[s].start    = time;
        current->slices[s].duration = 1;
        current->num_slices++;

        
        if (current->dynamic_priority > 0) {
            current->dynamic_priority--;
        }

        if (current->remaining_time == 0) {
            current->finish_time = time + 1;
            finished++;
        } else {

            if (current->quantum_left == 0) {
                current->quantum_left = AGING_QUANTUM;
            }

            int new_pr = current->dynamic_priority;
            if (new_pr < 0) new_pr = 0;
            if (new_pr > MAX_LEVELS) new_pr = MAX_LEVELS;

            enqueue(&queues[new_pr], current);
        }

        time++;
    }
}

