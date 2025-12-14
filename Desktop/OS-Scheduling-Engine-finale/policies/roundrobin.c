
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../includes/simulation.h"
#include "../includes/utils.h" 

void schedule_roundrobin(Process processes[], int num_processes, int quantum) {
    if (quantum <= 0) quantum = 4; 
    int current_time = 0;
    int completed_processes = 0;
    ProcessQueue ready_queue;
    init_queue(&ready_queue);
    
    qsort(processes, num_processes, sizeof(Process), compare_arrival_time);
    
    int next_arrival_index = 0;
    
    printf("--- Execution Round-Robin (Quantum: %d) ---\n", quantum);

    while (completed_processes < num_processes) {
        
        while (next_arrival_index < num_processes && 
            processes[next_arrival_index].arrival_time <= current_time) {
            enqueue(&ready_queue, &processes[next_arrival_index]);
            next_arrival_index++;
        }

        if (!is_empty(&ready_queue)) {
            Process *current_process = dequeue(&ready_queue);
            
            int execution_time = (current_process->remaining_time < quantum) ? 
                                current_process->remaining_time : quantum;
            
            if (current_process->is_started == 0) {
                current_process->start_time = current_time;
                current_process->is_started = 1;
                printf("Temps %d: Le processus '%s' demarre pour la premiere fois.\n", current_time, current_process->name);
            }

            if (current_process->num_slices < MAX_SLICES) {
                current_process->slices[current_process->num_slices].start = current_time;
                current_process->slices[current_process->num_slices].duration = execution_time;
                current_process->num_slices++;
            }

            current_time += execution_time;
            current_process->remaining_time -= execution_time;

            while (next_arrival_index < num_processes && 
                processes[next_arrival_index].arrival_time <= current_time) {
                enqueue(&ready_queue, &processes[next_arrival_index]);
                next_arrival_index++;
            }
            
            if (current_process->remaining_time == 0) {
                current_process->finish_time = current_time;
                completed_processes++;
                printf("Temps %d: Le processus '%s' termine.\n", current_time, current_process->name);
            } else {
                enqueue(&ready_queue, current_process);
                printf("Temps %d: '%s' preempte (restant: %d).\n", 
                    current_time, current_process->name, current_process->remaining_time);
            }
        } else {
            if (next_arrival_index < num_processes) {
                current_time = processes[next_arrival_index].arrival_time;
            } else {
                break;
            }
        }
    }
}