#include <stdio.h>
#include <stdlib.h>

#include "dyn_array.h"
#include "processing_scheduling.h"

#define FCFS "FCFS"
#define P "P"
#define RR "RR"
#define SJF "SJF"

void printResults(char* schedulingAlogorithm, ScheduleResult_t* results, FILE* fp);

int main(int argc, char **argv) {
    //make sure only 3 or 4 arguments are used
    if (argc < 3 || argc > 4) {
        printf("%s <pcb file> <schedule algorithm> [quantum]\n", argv[0]);
        return EXIT_FAILURE;
    }

    //input pcb filename from the command line
    char* inputFile = *(argv+1);
    //schedule algorith from the command line
    char* scheduleAlgorithm = *(argv+2);
    //quantum from the command line
    char* inputQuantum;
    //quantum to be passed to functions
    size_t quantum;
    //only want quantum if we are using round robin
    if(argc==4) {
        inputQuantum = *(argv+3);
        quantum = (size_t)atoi(inputQuantum);
    }

    int res = strcmp(scheduleAlgorithm, RR);
    
    //only want additional quantum arg if we are using round robin
    if(argc==4 && res!=0) {
        printf("\nQuantum only valid with round_robin\n");
        return EXIT_FAILURE;
    }

    //make sure that there is a quantum where rr is chosen
    if(strcmp(scheduleAlgorithm, RR)==0 && argc<4) {
        printf("\nGive your round robin a quantum\n");
        return EXIT_FAILURE;
    }

    //open the readme file
    FILE *fp;
    if(((fp=fopen("../readme.md", "a"))==NULL)) {
        printf("\nThere was an error opening the readme file\n");
        return EXIT_FAILURE;
    }

    //create the dyn_array with the processes from the input file from the command line
    dyn_array_t* dyn_array = load_process_control_blocks(inputFile);

    if(dyn_array==NULL) {
        printf("\nInvalid input file\n");
        fclose(fp);
        return EXIT_FAILURE;
    }
    
    //allocate the results structure and init
    ScheduleResult_t* results = malloc(sizeof(ScheduleResult_t));
    results->average_latency_time = 0;
    results->average_wall_clock_time = 0;
    results->total_run_time = 0;

    //call the given sorting algorithm given the request input from the command line and print results to the readme
    if(strcmp(scheduleAlgorithm, FCFS)==0) {
        first_come_first_serve(dyn_array, results);
        printResults(FCFS, results, fp);
    }
    else if(strcmp(scheduleAlgorithm, P)==0) {
        priority(dyn_array, results);
        printResults(P, results, fp);
    }
    else if(strcmp(scheduleAlgorithm, RR)==0) {
        round_robin(dyn_array, results, quantum);
        printResults(RR, results, fp);
    }
    else if(strcmp(scheduleAlgorithm, SJF)==0) {
        shortest_job_first(dyn_array, results);
        printResults(SJF, results, fp);

    }
    else {
        //the input was not a recognized scheduling algorithm
        printf("\nYour scheduling algorith was not recognized, select from this list:\n\tFCFS, P, RR, SJF\n");
        free(results);
        dyn_array_destroy(dyn_array);
        fclose(fp);
        return EXIT_FAILURE;
    }

    //free fp, results, and dyn_array
    free(results);
    dyn_array_destroy(dyn_array);
    fclose(fp);

    return EXIT_SUCCESS;
}

void printResults(char* schedulingAlogorithm, ScheduleResult_t* results, FILE* fp) {
    //print the results for the given scheduling algorithm passed to readme
    fprintf(fp, "%s %s%s", "\n\nScheduling results for", schedulingAlogorithm, ":");
    fprintf(fp, "%s %f", "\n\n\taverage_latency_time", results->average_latency_time);
    fprintf(fp, "%s %f", "\n\n\taverage_wall_clock_time", results->average_wall_clock_time);
    fprintf(fp, "%s %lu", "\n\n\ttotal_run_time", results->total_run_time);

    printf("%s %s%s", "\nScheduling results for", schedulingAlogorithm, ":");
    printf("%s %f", "\n\taverage_latency_time", results->average_latency_time);
    printf("%s %f", "\n\taverage_wall_clock_time", results->average_wall_clock_time);
    printf("%s %lu\n", "\n\ttotal_run_time", results->total_run_time);

}