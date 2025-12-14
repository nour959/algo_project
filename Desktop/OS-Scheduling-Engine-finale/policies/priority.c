
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> 

#include "../includes/simulation.h"
#include "../includes/utils.h"

void schedule_priority(Process processes[], int num_processes, int quantum) {
    int current_time = 0;
    int completed_processes = 0;

    qsort(processes, num_processes, sizeof(Process), compare_arrival_time);

    printf("--- Execution Priorite Preemptive (Priorites Croissantes: Plus Grande Valeur = Plus Haute Priorite) ---\n");

    while (completed_processes < num_processes) {
        
        Process *highest_priority_process = NULL;
        int max_priority = INT_MIN;  
        
        for (int i = 0; i < num_processes; i++) {
            Process *p = &processes[i];

            if (p->arrival_time <= current_time && p->remaining_time > 0) {
                if (p->priority > max_priority) {
                    max_priority = p->priority;
                    highest_priority_process = p;
                }
            }
        }

        if (highest_priority_process == NULL) {
            int next_arrival_time = INT_MAX;
            int next_arrival_index = -1;

            for (int i = 0; i < num_processes; i++) {
                if (processes[i].remaining_time > 0 && processes[i].arrival_time > current_time) {
                    if (processes[i].arrival_time < next_arrival_time) {
                        next_arrival_time = processes[i].arrival_time;
                        next_arrival_index = i;
                    }
                }
            }
            
            if (next_arrival_index != -1) {
                 printf("Temps %d: CPU inactif. Avance du temps à %d (jusqu'à '%s').\n", 
                        current_time, next_arrival_time, processes[next_arrival_index].name);
                current_time = next_arrival_time;
            } else {
                break;
            }
        } else {
            Process *current_process = highest_priority_process;

            if (current_process->is_started == 0) {
                current_process->start_time = current_time;
                current_process->is_started = 1;
                printf("Temps %d: Le processus '%s' demarre pour la premiere fois (Prio: %d).\n", 
                       current_time, current_process->name, current_process->priority);
            }

            int time_slice = current_process->remaining_time;
            
            int next_event_time = INT_MAX;
            for (int i = 0; i < num_processes; i++) {
                Process *p_next = &processes[i];
                
                if (p_next->arrival_time > current_time && 
                    p_next->arrival_time < next_event_time && 
                    p_next->priority > current_process->priority) 
                {
                    next_event_time = p_next->arrival_time;
                }
            }

            if (next_event_time != INT_MAX) {
                time_slice = (next_event_time - current_time) < time_slice ? 
                             (next_event_time - current_time) : time_slice;
            }

            if (current_process->num_slices < MAX_SLICES) {
                current_process->slices[current_process->num_slices].start = current_time;
                current_process->slices[current_process->num_slices].duration = time_slice;
                current_process->num_slices++;
                printf("DEBUG: Slice ajoutée pour '%s' (start=%d, dur=%d, num_slices=%d)\n", current_process->name, current_time, time_slice, current_process->num_slices);  // Debug temp
            } else {
                fprintf(stderr, "Avertissement: Limite de slices atteinte pour '%s'.\n", current_process->name);
            }

            current_time += time_slice;
            current_process->remaining_time -= time_slice;

            if (current_process->remaining_time == 0) {
                current_process->finish_time = current_time;
                completed_processes++;
                printf("Temps %d: Le processus '%s' termine.\n", current_time, current_process->name);
            } else if (current_time == next_event_time) {
                printf("Temps %d: Le processus '%s' est preempte par un nouvel arrivant de plus haute priorite.\n", current_time, current_process->name);
            } else {
            }
        }
    }
}