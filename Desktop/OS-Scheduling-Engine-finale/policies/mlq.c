
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../includes/simulation.h"
#include "../includes/utils.h"

#define MAX_QUEUES 10          
#define AGING_THRESHOLD 15     

typedef struct {
    ProcessQueue queue;
    int quantum;                
} PriorityQueue;

void schedule_mlq(Process processes[], int num_processes, int quantum) {
    if (quantum <= 0) quantum = 4;

    PriorityQueue queues[MAX_QUEUES];
    int max_priority = 0;
    int min_priority = INT_MAX;

    for (int i = 0; i < num_processes; i++) {
        if (processes[i].priority > max_priority) max_priority = processes[i].priority;
        if (processes[i].priority < min_priority) min_priority = processes[i].priority;
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].is_started = 0;
        processes[i].num_slices = 0;
        processes[i].aging_counter = 0;
    }

    for (int p = min_priority; p <= max_priority && p < MAX_QUEUES; p++) {
        init_queue(&queues[p].queue);
        queues[p].quantum = quantum;  
    }

    int current_time = 0;
    int completed = 0;
    int next_arrival_idx = 0;

    printf("--- Execution Multi-Level Queue (Quantum: %d, Aging: %d) ---\n", quantum, AGING_THRESHOLD);

    while (completed < num_processes) {

        while (next_arrival_idx < num_processes && 
               processes[next_arrival_idx].arrival_time <= current_time) {
            Process *p = &processes[next_arrival_idx];
            int qidx = p->priority;
            if (qidx >= MAX_QUEUES) qidx = MAX_QUEUES - 1;
            enqueue(&queues[qidx].queue, p);
            next_arrival_idx++;
        }


        Process *current = NULL;
        int selected_queue = -1;
        int execution_time = 0;

        for (int p = max_priority; p >= min_priority; p--) {
            if (p >= MAX_QUEUES) continue;
            if (!is_empty(&queues[p].queue)) {
                current = queues[p].queue.elements[queues[p].queue.front];
                selected_queue = p;
                break;
            }
        }

        if (current == NULL) {
            
            if (next_arrival_idx < num_processes) {
                current_time = processes[next_arrival_idx].arrival_time;
            } else {
                break;
            }
            continue;
        }

    
        int time_slice = queues[selected_queue].quantum;
        if (current->remaining_time < time_slice) {
            time_slice = current->remaining_time;
        }

        int next_higher_arrival = INT_MAX;
        for (int i = next_arrival_idx; i < num_processes; i++) {
            Process *p = &processes[i];
            if (p->arrival_time > current_time && 
                p->arrival_time < current_time + time_slice &&
                p->priority > current->priority) {
                if (p->arrival_time < next_higher_arrival) {
                    next_higher_arrival = p->arrival_time;
                }
            }
        }

        if (next_higher_arrival != INT_MAX) {
            time_slice = next_higher_arrival - current_time;
        }

        if (current->is_started == 0) {
            current->start_time = current_time;
            current->is_started = 1;
            printf("Temps %d: '%s' démarre (Prio: %d)\n", current_time, current->name, current->priority);
        }

        if (current->num_slices < MAX_SLICES) {
            current->slices[current->num_slices].start = current_time;
            current->slices[current->num_slices].duration = time_slice;
            current->num_slices++;
        }

        current_time += time_slice;
        current->remaining_time -= time_slice;

        if (current->remaining_time > 0) {
            dequeue(&queues[selected_queue].queue);  
            enqueue(&queues[current->priority].queue, current);  
        } else {
            dequeue(&queues[selected_queue].queue);
            current->finish_time = current_time;
            completed++;
            printf("Temps %d: '%s' termine.\n", current_time, current->name);
        }
    }
}