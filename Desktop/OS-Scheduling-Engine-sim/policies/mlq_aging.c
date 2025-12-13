#include <stdio.h>
#include <stdlib.h>

#include "../includes/process.h"
#include "../includes/simulation.h"
#include "../includes/utils.h"

// Quantum fixe pour Round Robin dans un niveau.
// On ne lit pas le "quantum" passé par l'IHM, on garde 2 ms fixes ici.
#define AGING_QUANTUM 2

// Maximum raisonnable de niveaux de priorité dynamiques
#define MAX_LEVELS 10

// Prototype attendu par le reste du projet : 3 paramètres.
void schedule_mlq_aging(Process processes[], int num_processes, int quantum_unused)
{
    (void)quantum_unused;  // évite un warning "unused parameter"

    ProcessQueue queues[MAX_LEVELS + 1];
    int time = 0;
    int finished = 0;

    // -----------------------------
    // Initialisation des files
    // -----------------------------
    for (int i = 0; i <= MAX_LEVELS; i++) {
        init_queue(&queues[i]);
    }

    // -----------------------------
    // Initialisation des processus
    // -----------------------------
    for (int i = 0; i < num_processes; i++) {
        Process *p = &processes[i];

        p->remaining_time   = p->burst_time;
        p->dynamic_priority = p->priority;      // priorité dynamique = statique au début
        p->quantum_left     = AGING_QUANTUM;    // quantum interne pour RR

        p->start_time  = -1;
        p->finish_time = 0;

        p->num_slices  = 0;
        p->is_started  = 0;                     // si ce champ existe dans Process
    }

    // -----------------------------
    // Boucle principale de simulation
    // -----------------------------
    while (finished < num_processes) {

        // 1) Ajouter les processus qui arrivent à "time"
        for (int i = 0; i < num_processes; i++) {
            if (processes[i].arrival_time == time) {
                int pr = processes[i].dynamic_priority;
                if (pr < 0) pr = 0;
                if (pr > MAX_LEVELS) pr = MAX_LEVELS;

                enqueue(&queues[pr], &processes[i]);
            }
        }

        // 2) Trouver le niveau de priorité le plus élevé non vide
        int lvl = -1;
        for (int p = MAX_LEVELS; p >= 0; p--) {
            if (!is_empty(&queues[p])) {
                lvl = p;
                break;
            }
        }

        // Si aucune file n'a de processus prêt → CPU idle
        if (lvl == -1) {
            time++;
            continue;
        }

        // 3) Récupérer le premier processus de ce niveau
        Process *current = dequeue(&queues[lvl]);

        if (current->start_time == -1) {
            current->start_time = time;
        }

        // 4) Exécuter le processus pendant 1 ms
        current->remaining_time--;
        current->quantum_left--;

        // Enregistrer le slice pour l'affichage Gantt
        int s = current->num_slices;
        current->slices[s].start    = time;
        current->slices[s].duration = 1;
        current->num_slices++;

        // RÈGLE D’AGING :
        // À chaque fois qu’un processus s’exécute,
        // sa priorité dynamique diminue de 1 (sans aller sous 0).
        if (current->dynamic_priority > 0) {
            current->dynamic_priority--;
        }

        // 5) Tester la fin du processus
        if (current->remaining_time == 0) {
            // Le processus se termine à la fin du slice [time, time+1)
            current->finish_time = time + 1;
            finished++;
        } else {
            // Sinon, on le réinsère dans la file appropriée.

            // Si le quantum interne est épuisé → RR : on réinitialise
            if (current->quantum_left == 0) {
                current->quantum_left = AGING_QUANTUM;
            }

            int new_pr = current->dynamic_priority;
            if (new_pr < 0) new_pr = 0;
            if (new_pr > MAX_LEVELS) new_pr = MAX_LEVELS;

            enqueue(&queues[new_pr], current);
        }

        // 6) Avancer le temps global d'une unité
        time++;
    }
}

