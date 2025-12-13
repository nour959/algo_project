// src/utils.c

#include "../includes/utils.h"
#include <stdlib.h> 
#include <string.h> 
#include <stdio.h> // Pour fprintf

// ----------------------------------------------------
// Fonctions de Comparaison
// ----------------------------------------------------

// Tri par temps d'arrivée croissant.
int compare_arrival_time(const void *a, const void *b) {
    const Process *proc_a = (const Process *)a;
    const Process *proc_b = (const Process *)b;

    if (proc_a->arrival_time != proc_b->arrival_time) {
        return proc_a->arrival_time - proc_b->arrival_time;
    }
    // Si les temps d'arrivée sont égaux, trier par nom
    return strcmp(proc_a->name, proc_b->name);
}

// Tri par priorité croissante (le plus petit est prioritaire).
int compare_priority(const void *a, const void *b) {
    const Process *proc_a = (const Process *)a;
    const Process *proc_b = (const Process *)b;

    if (proc_a->priority != proc_b->priority) {
        return proc_a->priority - proc_b->priority;
    }
    // Si les priorités sont égales, trier par temps d'arrivée
    return compare_arrival_time(a, b);
}

// ----------------------------------------------------
// Fonctions de File d'Attente (Queue)
// ----------------------------------------------------

void init_queue(ProcessQueue *q) {
    q->front = -1;
    q->rear = -1;
}

int is_empty(ProcessQueue *q) {
    return q->front == -1;
}

// NOTE: Ajout de is_full (non prototypé dans utils.h mais utile ici)
int is_full(ProcessQueue *q) {
    return q->rear == MAX_PROCESSES - 1; 
}

void enqueue(ProcessQueue *q, Process *p) {
    if (is_full(q)) {
        fprintf(stderr, "Erreur: File pleine. Impossible d'ajouter le processus %s.\n", p->name);
        return;
    }
    if (is_empty(q)) {
        q->front = 0;
    }
    q->rear++;
    q->elements[q->rear] = p;
}

Process* dequeue(ProcessQueue *q) {
    if (is_empty(q)) {
        return NULL;
    }
    
    Process *p = q->elements[q->front];
    
    // Décalage des éléments (Simple Queue implementation)
    for (int i = q->front; i < q->rear; i++) {
        q->elements[i] = q->elements[i + 1];
    }
    
    q->rear--;
    
    if (q->rear == -1) {
        q->front = -1;
    }
    return p;
}