# RTOS-Round-Robin-CPU-Scheduling
Real-time Operating System CPU scheduling by using the Round-Robin (RR) algorithm

* To compile the program, please ensure that gcc is installed and run the following command:
```
gcc rrcpusched.c -o rrcpusched -pthread -lrt
```

Program execution:
* The program will ask for a quantum time in order to schedule the CPU, where the quantum time indicates how long the process can be executed in the current round
* When the process finishes the quantum,
** If process has finish all its brust time, go to the complete list
** Else, process will go around until all its brust time finished then go to the complete list
* The program will generate a text called output.txt which contains the average waiting time and average turnaround time of the RR CPU scheduling

More about Round-Robin CPU scheduling: https://en.wikipedia.org/wiki/Round-robin_scheduling 
