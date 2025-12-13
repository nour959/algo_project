// includes/utils.h

#ifndef UTILS_H
#define UTILS_H

#include "process.h"
#include <string.h> 

// Fonction de comparaison pour qsort (tri par temps d'arrivée)
int compare_arrival_time(const void *a, const void *b);

// Fonction de comparaison pour qsort (tri par priorité, les plus petites valeurs sont prioritaires)
int compare_priority(const void *a, const void *b);

#endif // UTILS_H