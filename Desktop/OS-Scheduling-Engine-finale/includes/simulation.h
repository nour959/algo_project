
#ifndef SIMULATION_H
#define SIMULATION_H

#include "process.h"

typedef void (*SchedulerFunc)(Process[], int, int); 

void run_simulation(Process processes[], int num_processes, SchedulerFunc scheduler, int quantum);
void display_results(Process processes[], int num_processes);
void schedule_mlq_aging(Process processes[], int num_processes, int quantum);

#endif 