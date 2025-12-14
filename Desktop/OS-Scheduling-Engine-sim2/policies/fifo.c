
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../includes/simulation.h"
#include "../includes/utils.h"

void schedule_fifo(Process processes[], int num_processes, int quantum) {
    qsort(processes, num_processes, sizeof(Process), compare_arrival_time);

    int current_time = 0; 

    printf("--- Execution FIFO ---\n");

    for (int i = 0; i < num_processes; i++) {
        Process *p = &processes[i];

        if (current_time < p->arrival_time) {
            current_time = p->arrival_time;
        }

        p->start_time = current_time;
        p->is_started = 1;

        if (p->num_slices < MAX_SLICES) {
            p->slices[p->num_slices].start = current_time;
            p->slices[p->num_slices].duration = p->burst_time;
            p->num_slices++;
        }

        current_time += p->burst_time;
        p->finish_time = current_time;
        p->remaining_time = 0; 

        printf("Temps %d: Le processus '%s' demarre.\n", p->start_time, p->name);
        printf("Temps %d: Le processus '%s' termine.\n", p->finish_time, p->name);
    }
}