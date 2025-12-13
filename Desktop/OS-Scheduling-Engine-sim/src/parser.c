// src/parser.c

#include "../includes/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 256

int parse_processes(const char* filename, Process processes[], int* num_processes) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier de configuration");
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    *num_processes = 0;

    while (fgets(line, sizeof(line), file)) {
        // Suppression des commentaires
        char *comment_hash = strchr(line, '#');
        if (comment_hash) *comment_hash = '\0';
        char *comment_slash = strstr(line, "//");
        if (comment_slash) *comment_slash = '\0';

        // Vérification des lignes blanches
        char temp_line[MAX_LINE_LENGTH];
        strcpy(temp_line, line);
        char *ptr = temp_line;
        while (*ptr && isspace((unsigned char)*ptr)) {
            ptr++; 
        }
        if (*ptr == '\0') {
            continue; 
        }

        // Extraction des données
        if (*num_processes < MAX_PROCESSES) {
            int items_read = sscanf(
                line, 
                "%s %d %d %d", 
                processes[*num_processes].name,
                &processes[*num_processes].arrival_time, 
                &processes[*num_processes].burst_time, 
                &processes[*num_processes].priority
            );

            if (items_read == 4) {
                processes[*num_processes].remaining_time = processes[*num_processes].burst_time;
                processes[*num_processes].start_time = -1; 
                processes[*num_processes].is_started = 0; 
                processes[*num_processes].num_slices = 0;  // Nouveau: Initialisation des slices
                processes[*num_processes].queue_level = 0; // Initialisation MLQ
                processes[*num_processes].aging_counter = 0; // Initialisation Aging
                (*num_processes)++;
            } else if (items_read > 0) {
                 fprintf(stderr, "Avertissement: Ligne ignorée (format incorrect): %s", line);
            }
        } else {
            fprintf(stderr, "Erreur: Nombre maximal de processus atteint (%d).\n", MAX_PROCESSES);
            break;
        }
    }

    fclose(file);
    return 0;
}