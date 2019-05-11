#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dyn_array.h"
#include "processing_scheduling.h"


// private function
void virtual_cpu(ProcessControlBlock_t *process_control_block) {
   if(process_control_block==NULL) {
       return;
   }

    // decrement the burst time of the pcb
    --process_control_block->remaining_burst_time;
}

//this function is a comparator on the priority field in ProcessControlBlock_t
int priorityComparator(const void *x, const void *y) { 
    //we want to sort descending so if x > y return a value less than one
    if( ((ProcessControlBlock_t*)x)->priority > ((ProcessControlBlock_t*)y)->priority ) {
        return -1;
    }
    else if( ((ProcessControlBlock_t*)x)->priority < ((ProcessControlBlock_t*)y)->priority ) {
        return 1;
    }
    
    return 0;
}

//this function is a comparator on the burst time field in ProcessControlBlock_t
int shortestComparator(const void *x, const void *y) { 
    //we want to sort descending so if x > y return a value less than one
    if( ((ProcessControlBlock_t*)x)->remaining_burst_time > ((ProcessControlBlock_t*)y)->remaining_burst_time ) {
        return -1;
    }
    else if( ((ProcessControlBlock_t*)x)->remaining_burst_time < ((ProcessControlBlock_t*)y)->remaining_burst_time ) {
        return 1;
    }
    
    return 0;
}

//this function uses the fcfs scheduling algorithm to scedule processes on the virtual cpu
bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) {
    //make sure input is valid
    if(ready_queue==NULL || result==NULL) {
        return false;
    }

    //init
    result->total_run_time=0;
    //bool to tell if extraction from dyn_array was successful
    bool extracted = false;

    //counter
    uint32_t i = 0;
    //size of the dyn_array
    uint32_t size = dyn_array_size(ready_queue);
    //will help keep track of the time for average wall clock time
    uint32_t currentTime = 0;
    //keeps tract of the total latency time
    uint32_t totalLatent = 0;

    //loop over dyn_array and extract pcb and run on the cpu
    //extract the first process in the queue and run until it is done, following fcfs
    for(i=0; i<size; i++) {
        ProcessControlBlock_t pcb;
    
        //make sure extraction success
        extracted = dyn_array_extract_back(ready_queue, &pcb);
        if(extracted == false) {
            return false;
        }

        //running total of latency, since fcfs it will be very closely related to 
        //total time, except for the first process scheduled
        totalLatent += result->total_run_time;

        //run the process until it is finished on the virtual cpu
        pcb.started = true;
        while(pcb.remaining_burst_time != 0) {
            virtual_cpu(&pcb);
            currentTime++;
            result->total_run_time++;
        }

        //process is guaranteed to be completed after being run to completed fcfs
        pcb.started = false;

        //average wall clock time is the cumulative total time of all processes until they finish
        result->average_wall_clock_time+=currentTime;
    }

    //average out the results
    result->average_latency_time = (float)totalLatent/size;
    result->average_wall_clock_time = (float)result->average_wall_clock_time/size;

    return true;
}

//this function uses the fcfs scheduling algorithm to scedule processes on the virtual cpu
bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) {
    //input validation
    if(ready_queue==NULL || result==NULL) {
        return false;
    }

    //since using priority as our scheduling algorithm, make sure to sort on priority before extracting from the queue
    dyn_array_sort(ready_queue, priorityComparator);

    //init
    result->total_run_time=0;
    //bool to tell if extraction from dyn_array was successful
    bool extracted = false;

    //counter
    uint32_t i = 0;
    //size of dyn array
    uint32_t size = dyn_array_size(ready_queue);
    //will help keep track of the time for average wall clock time
    uint32_t currentTime = 0;
    //keeps tract of the total latency time
    uint32_t totalLatent = 0;

    //loop over dyn_array and extract pcb and run on the cpu
    //extract the first process in the queue and run until it is done, following priority
    for(i=0; i<size; i++) {
        ProcessControlBlock_t pcb;
    
        //make sure extraction success
        extracted = dyn_array_extract_back(ready_queue, &pcb);
        if(extracted == false) {
            return false;
        }

        //running total of latency, since priority it will be very closely related to 
        //total time, except for the first process scheduled
        totalLatent += result->total_run_time;

        //run the process until it is finished on the virtual cpu
        pcb.started = true;
        while(pcb.remaining_burst_time != 0) {
            virtual_cpu(&pcb);
            currentTime++;
            result->total_run_time++;
        }
        
        //process is guaranteed to be completed after being run to completed fcfs
        pcb.started = false;

        //average wall clock time is the cumulative total time of all processes until they finish
        result->average_wall_clock_time+=currentTime;
    }

    //average out the results
    result->average_latency_time = (float)totalLatent/size;
    result->average_wall_clock_time = (float)result->average_wall_clock_time/size;

    return true;  
}

//this function uses the round robin scheduling algorithm to scedule processes on the virtual cpu
bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) {
    //validate input
    if(ready_queue==NULL || result==NULL || quantum == 0) {
        return false;
    }

    //init
    result->total_run_time=0;
    //bool to tell if extraction from dyn_array was successful
    bool extracted = false;

    //size of the dyn_array
    uint32_t size = dyn_array_size(ready_queue);
    //counter to make sure that we only run a process on the virtual cpu
    //for a maximum to quantum time
    uint32_t counter = quantum;
    //init
    result->average_latency_time = 0;

    //loop over dyn_array and extract pcb and run on the cpu as long as their are still processes to be run
    //extract the first process in the queue and run until it is done, or reaches the quantum, following rr
    while(dyn_array_size(ready_queue)!=0) {
        ProcessControlBlock_t pcb;
    
        //make sure extraction success
        extracted = dyn_array_extract_back(ready_queue, &pcb);
        if(extracted == false) {
            return false;
        }

        //run the process until it is finished on the virtual cpu or until it reaches the quantum or time on the cpu
        //if so, put back in queue and run the next process
        pcb.started = true;
        while(pcb.remaining_burst_time != 0 && counter!=0) {
            virtual_cpu(&pcb);
            result->total_run_time++;
            //allows the process to run for only max of quantum time
            counter--;
            //the latency is the amount of processes that still remain in the queue for each "clock tick"
            result->average_latency_time += dyn_array_size(ready_queue);
        }

        //mark the process finished if it is done executing
        //put back in the queue if it still has time remaining but exceeded the queue time
        if(pcb.remaining_burst_time == 0) {
            pcb.started = false;
            //add on the total time from arrival (time zero) to finish
            result->average_wall_clock_time += result->total_run_time;
        }
        else {
            //put on the other end of the queue to give other processes a chance
            dyn_array_push_front(ready_queue, &pcb);
        }

        //reset counter
        counter = quantum;
    }

    //average results
    result->average_latency_time = (float)result->average_latency_time/size;
    result->average_wall_clock_time = (float)result->average_wall_clock_time/size;

    return true;
}

//this function takes the given inputfile of pcbs and loads them into a dyn_array
dyn_array_t *load_process_control_blocks(const char *input_file) {
    //check for valid input
    if(input_file==NULL) {
        return NULL;
    }

    //open the file
    int file = open(input_file, O_RDONLY);

    //make sure file opened correctly
    if(file<0) {
        return NULL;
    }

    //first uint32 is numProcess
    uint32_t numProcess = 0;
    read(file, &numProcess, sizeof(uint32_t));

    //make sure the file is not empty and has valid numProcesses
    if(numProcess<=0) {
        return NULL;
    }

    //create the dyn array, holds ProcessControlBlock_t, will resize as we add to it
    dyn_array_t* dyn_array = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);

    //burst and priority to read in
    uint32_t burst = 0;
    uint32_t priority = 0;

    //checks for success of reads
    int didRead = 0;

    //counter
    uint32_t i = 0;
    for(i=0; i<numProcess; i++) {
        //make sure that there are pairs of bursts/priorities
        didRead = read(file, &burst, sizeof(uint32_t));
        if(didRead<=0) {
            dyn_array_destroy(dyn_array);
            return NULL;
        }
        didRead = read(file, &priority, sizeof(uint32_t));
        if(didRead<=0) {
            dyn_array_destroy(dyn_array);
            return NULL;
        }

        //load up pcb to be instered to dyn array
        ProcessControlBlock_t pcb;
        pcb.remaining_burst_time = burst;
        pcb.priority = priority;
        pcb.started = false;

        dyn_array_push_back(dyn_array, &pcb);
    }

    return dyn_array;
}

//this function uses the sjf scheduling algorithm to scedule processes on the virtual cpu
bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) { 
    //input validation
    if(ready_queue==NULL || result==NULL) {
        return false;
    }

    //sort the queue based on the remaining burst time, since sjf
    dyn_array_sort(ready_queue, shortestComparator);

    //init
    result->total_run_time=0;
    //bool to tell if extraction from dyn_array was successful
    bool extracted = false;

    //counter
    uint32_t i = 0;
    //size of the dyn array
    uint32_t size = dyn_array_size(ready_queue);

    //will help keep track of the time for average wall clock time
    uint32_t currentTime = 0;
    //keeps tract of the total latency time
    uint32_t totalLatent = 0;

    //loop over dyn_array and extract pcb and run on the cpu
    //extract the first process in the queue and run until it is done, following fcfs
    for(i=0; i<size; i++) {
        ProcessControlBlock_t pcb;

        //make extraction success
        extracted = dyn_array_extract_back(ready_queue, &pcb);
        if(extracted == false) {
            return false;
        }

        //running total of latency, since fcfs it will be very closely related to 
        //total time, except for the first process scheduled
        totalLatent += result->total_run_time;

        //run the process until it is finished on the virtual cpu
        pcb.started = true;
        while(pcb.remaining_burst_time != 0) {
            virtual_cpu(&pcb);
            currentTime++;
            result->total_run_time++;
        }

        //process is guaranteed to be completed after being run to completed fcfs
        pcb.started = false;

        //average wall clock time is the cumulative total time of all processes until they finish
        result->average_wall_clock_time+=currentTime;
    }

    //average the results
    result->average_latency_time = (float)totalLatent/size;
    result->average_wall_clock_time = (float)result->average_wall_clock_time/size;

    return true; 
}