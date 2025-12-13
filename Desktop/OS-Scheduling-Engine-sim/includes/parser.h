// includes/parser.h

#ifndef PARSER_H
#define PARSER_H

#include "process.h"

int parse_processes(const char* filename, Process processes[], int* num_processes);

#endif // PARSER_H