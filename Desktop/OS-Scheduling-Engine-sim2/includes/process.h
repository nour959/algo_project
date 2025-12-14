
#ifndef PROCESS_H
#define PROCESS_H

#define MAX_NAME_LENGTH 50
#define MAX_PROCESSES 100 
#define MAX_SLICES 50     

typedef struct {
    char name[MAX_NAME_LENGTH]; 
    int arrival_time;          
    int burst_time;           
    int priority;             

    int remaining_time;        
    int start_time;            
    int finish_time;           
    int waiting_time;          
    int turnaround_time;       
    
    int is_started; 

    struct {
        int start;    
        int duration;  
    } slices[MAX_SLICES];
    int num_slices;  

    int queue_level;    
    int aging_counter;  
    int dynamic_priority;
    int quantum_left;


} Process;


typedef struct {
    Process* elements[MAX_PROCESSES]; 
    int front; 
    int rear;  
} ProcessQueue;

void init_queue(ProcessQueue *q);
int is_empty(ProcessQueue *q);
void enqueue(ProcessQueue *q, Process *p);
Process* dequeue(ProcessQueue *q);

#endif 