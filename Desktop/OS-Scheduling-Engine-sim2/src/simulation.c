
#include "../includes/simulation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


void display_results(Process processes[], int num_processes) {
    
    printf("\n--- RESULTATS DE LA SIMULATION ---\n");

    float total_waiting_time = 0;
    float total_turnaround_time = 0;

    printf("\n| %-10s | %-8s | %-8s | %-8s | %-8s | %-14s | %-12s |\n",
           "Processus", "Arrivee", "Execution", "Debut", "Fin", "Temps Rotation", "Temps Attente");
    printf("|------------|----------|-----------|----------|----------|----------------|--------------|\n");

    for (int i = 0; i < num_processes; i++) {
        processes[i].turnaround_time = processes[i].finish_time - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
        
        total_waiting_time += processes[i].waiting_time;
        total_turnaround_time += processes[i].turnaround_time;

        printf("| %-10s | %-8d | %-8d | %-8d | %-8d | %-14d | %-12d |\n",
               processes[i].name,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].start_time,
               processes[i].finish_time,
               processes[i].turnaround_time,
               processes[i].waiting_time);
    }
    printf("---------------------------------------------------------------------------------------\n");

    printf("Temps d'Attente Moyen: %.2f\n", total_waiting_time / num_processes);
    printf("Temps de Rotation Moyen: %.2f\n", total_turnaround_time / num_processes);
}


void run_simulation(Process processes[], int num_processes, SchedulerFunc scheduler, int quantum) {
    if (scheduler != NULL) {
        scheduler(processes, num_processes, quantum);
    }
}