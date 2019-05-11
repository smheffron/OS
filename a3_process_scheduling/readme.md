# Assignment 3

Milestone 0: CMake and Unit Testing filled out 
Due: Wednesday, February 27, 2019 11:59 PM

Milestone 1: PCB file loading and First Come, First Served 
Due: Wednesday, March 6, 2019 11:59 PM

Milestone 2: Priority scheduling, Round Robin, and analysis of algorithms 
Due: Wednesday, March 13, 2019 11:59 PM

Note: 
1. After you run assignment3_test, the PCBs.bin will be created in the ./test directory.
   so in the ./build directory, you can run the analysis program like "./analysis ../test/PCBs.bin RR 4"; or you can simply copy and paste the PCBs.bin to the ./build directory, and then run "./analysis PCBs.bin RR 4".
     
2. You can manually copy the time analysis from console and paste it to this readme file, but directly output from your program is strongly recommended.     
    ---------------------------------------------------------------------------
    Add your scheduling algorithm analysis below this line in a readable format. 
    ---------------------------------------------------------------------------


Scheduling results for P:

	average_latency_time 111.199997

	average_wall_clock_time 118.766670

	total_run_time 227

Scheduling results for FCFS:

	average_latency_time 108.233330

	average_wall_clock_time 115.800003

	total_run_time 227

Scheduling results for RR:

	average_latency_time 138.100006

	average_wall_clock_time 145.666672

	total_run_time 227

Scheduling results for SJF:

	average_latency_time 74.133331

	average_wall_clock_time 81.699997

	total_run_time 227