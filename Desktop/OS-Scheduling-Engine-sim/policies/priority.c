// policies/priority.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> 

#include "../includes/simulation.h"
#include "../includes/utils.h"

void schedule_priority(Process processes[], int num_processes, int quantum) {
    // 1. Initialisation
    int current_time = 0;
    int completed_processes = 0;

    // Trier initialement par temps d'arrivée pour gérer l'arrivée dans l'ordre
    qsort(processes, num_processes, sizeof(Process), compare_arrival_time);

    printf("--- Execution Priorite Preemptive (Priorites Croissantes: Plus Grande Valeur = Plus Haute Priorite) ---\n");

    // 2. Boucle principale de la simulation
    while (completed_processes < num_processes) {
        
        // 2a. Identifier le processus 'prêt' avec la plus haute priorité (plus grande valeur)
        Process *highest_priority_process = NULL;
        int max_priority = INT_MIN;  // Chercher le maximum au lieu de minimum
        
        // Parcourir tous les processus pour trouver celui de plus haute priorité arrivé
        for (int i = 0; i < num_processes; i++) {
            Process *p = &processes[i];

            // Le processus doit être arrivé et ne doit pas être terminé
            if (p->arrival_time <= current_time && p->remaining_time > 0) {
                // Priorité statique: la valeur la plus grande est la plus haute priorité
                if (p->priority > max_priority) {
                    max_priority = p->priority;
                    highest_priority_process = p;
                }
                // Règle de départage : si priorités égales, FIFO (géré par le tri initial)
            }
        }

        // 2b. Gérer l'inactivité du CPU (Idle)
        if (highest_priority_process == NULL) {
            int next_arrival_time = INT_MAX;
            int next_arrival_index = -1;

            // Trouver l'arrivée du prochain processus non traité
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
                // Tous les processus sont terminés
                break;
            }
        } else {
            // 2c. Exécuter le processus de plus haute priorité
            Process *current_process = highest_priority_process;

            // Début d'exécution (seulement la première fois)
            if (current_process->is_started == 0) {
                current_process->start_time = current_time;
                current_process->is_started = 1;
                printf("Temps %d: Le processus '%s' demarre pour la premiere fois (Prio: %d).\n", 
                       current_time, current_process->name, current_process->priority);
            }

            // Déterminer le temps d'exécution (jusqu'à la prochaine arrivée ou la fin du processus)
            int time_slice = current_process->remaining_time;
            
            // Trouver l'événement de temps suivant (arrivée d'un processus de plus haute priorité)
            int next_event_time = INT_MAX;
            for (int i = 0; i < num_processes; i++) {
                Process *p_next = &processes[i];
                
                // Si un autre processus arrive plus tard mais avec une meilleure priorité (plus grande)
                if (p_next->arrival_time > current_time && 
                    p_next->arrival_time < next_event_time && 
                    p_next->priority > current_process->priority) 
                {
                    next_event_time = p_next->arrival_time;
                }
            }

            // Le temps d'exécution est limité soit par le temps restant, soit par la prochaine arrivée préemptive
            if (next_event_time != INT_MAX) {
                time_slice = (next_event_time - current_time) < time_slice ? 
                             (next_event_time - current_time) : time_slice;
            }

            // NOUVEAU: Enregistrer la tranche d'exécution (slice) pour le Gantt segmenté
            if (current_process->num_slices < MAX_SLICES) {
                current_process->slices[current_process->num_slices].start = current_time;
                current_process->slices[current_process->num_slices].duration = time_slice;
                current_process->num_slices++;
                printf("DEBUG: Slice ajoutée pour '%s' (start=%d, dur=%d, num_slices=%d)\n", current_process->name, current_time, time_slice, current_process->num_slices);  // Debug temp
            } else {
                fprintf(stderr, "Avertissement: Limite de slices atteinte pour '%s'.\n", current_process->name);
            }

            // 2d. Simulation de l'exécution
            current_time += time_slice;
            current_process->remaining_time -= time_slice;

            // 2e. Gérer la terminaison ou la préemption
            if (current_process->remaining_time == 0) {
                current_process->finish_time = current_time;
                completed_processes++;
                printf("Temps %d: Le processus '%s' termine.\n", current_time, current_process->name);
            } else if (current_time == next_event_time) {
                // Préemption par un nouvel arrivant
                 printf("Temps %d: Le processus '%s' est preempte par un nouvel arrivant de plus haute priorite.\n", current_time, current_process->name);
            } else {
                // Le processus continue son exécution jusqu'à la prochaine itération
            }
        }
    }
}