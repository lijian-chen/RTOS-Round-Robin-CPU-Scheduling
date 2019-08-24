/* 
 * Project: Round Robin CPU Scheduling simulation
 *
 * Author: Lijian Chen
 */

/*
 * Compilation: gcc rrcpusched.c -o rrcpusched -pthread -lrt

 * The rrcpusched.c will generate a file called "output.txt" which contained the 
 *     average waiting time and average turnaround time.
 */

#include <pthread.h> 	/* pthread functions and data structures for pipe */
#include <unistd.h> 	/* for POSIX API */
#include <stdlib.h> 	/* for exit() function */
#include <stdio.h>		/* standard I/O routines */
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define PROCESS_NUM 7

/* Structures */
typedef struct Process {
  int pid; // Process ID
  int just_arrived; // Just arrive flag
  int arrive_t, wait_t, burst_t, turnaround_t, remain_burst_t, execute_t, total_burst_t; // Process time
} Process;

typedef struct RR_Params {
  int total_t, quantum_t;
} RR_Params;

/* Global Variables */
int i, fifo_count = 0; // Index, FIFO counter
Process processes[PROCESS_NUM], fifo[PROCESS_NUM];
sem_t sem_RR; // Semaphore
pthread_t tid[2]; // Pthreads
float avg_wait_t, avg_turnaround_t; // Average wait time and average turnaround time

/* Prototypes */
void RR_CPU_Scheduling(RR_Params rr_params);
void initializeProcesses();
void appendToList(Process process, Process fifo[], int index);
void shiftToLeft(Process fifo[]);
void deleteFromList(Process fifo[]);
float getAverageWaitingTime(Process fifo[]);
float getAverageTurnaroundTime(Process fifo[]);
void send_FIFO();
void read_FIFO();

/* this function calculates CPU RR scheduling, writes waiting time and turn-around time to th FIFO */
void *worker1(void *params)
{
  RR_Params *RR_P = params;
  
  // Testing message
  printf("[Thread1 Message] - Hello from worker1!\n");
  initializeProcesses();
  
  RR_CPU_Scheduling(*RR_P);
  avg_wait_t = getAverageWaitingTime(fifo);
  avg_turnaround_t = getAverageTurnaroundTime(fifo);
  printf("The average waiting time is %f, average turnaround time is %f\n", avg_wait_t, avg_turnaround_t);
  printf("Round Robin CPU Scheduling completed!\n");

  send_FIFO();
}

/* reads the waiting time and turn-around time through the FIFO and writes to text file */
void *worker2()
{
  // Wait until the sem_RR finish
  sem_wait(&sem_RR);

  // Testing message
  printf("\n[Thread2 Message] - Hello from worker2!\n");

  read_FIFO();
}

/* this main function creates named pipe and threads */
int main(void)
{
  int err;
  pthread_attr_t attr;
  RR_Params params;

  /* creating a named pipe(FIFO) with read/write permission */
  // add your code 

  /* initialize the parameters */
  printf("Please enter the quantum time for the Round Robin CPU Scheduling: ");
  scanf("%d", &params.quantum_t);

  avg_wait_t = 0.0;  
  avg_turnaround_t = 0.0;

  if(sem_init(&sem_RR, 0, 0) != 0)
  {
    perror("Error initializing semaphore: ");
    exit(-1);
  }

  pthread_attr_init(&attr);
	
  /* create threads */
  if(err = pthread_create(&(tid[0]), &attr, &worker1, (void*)(&params)))
  {
    perror("Error creating threads: ");
    exit(-1);
  }

  if(err = pthread_create(&(tid[1]), &attr, &worker2, (void*)(&params)))
  {
    perror("Error creating threads: ");
    exit(-1);
  }
	
  /* wait for the thread to exit */
  if(err = pthread_join(tid[0], NULL))
  {
    perror("Error joining threads: ");
    exit(-1);
  }

  if(err = pthread_join(tid[1], NULL))
  {
    perror("Error joining threads: ");
    exit(-1);
  }
	
  return 0;
}

void RR_CPU_Scheduling(RR_Params rr_params) 
{
  int timer, loop_count = 0, fifo_checker = 0, completed_count = 0;
  Process completedList[PROCESS_NUM];
  
  printf("\nStart Round Robin CPU Scheduling...\n");
  printf("Total burst time is %d\n", processes->total_burst_t);
  // The total time calculated by the total burst time 
  // plus the waiting time before the first process arrived
  rr_params.total_t = 8 + processes->total_burst_t;
  printf("Total time is %d\n", rr_params.total_t);
  printf("Quantum time is %d\n\n", rr_params.quantum_t);

  // Initialize the timer to 0
  for(timer = 0; timer <= rr_params.total_t; timer++)
  {
    // Keep tracking the current time
    printf("Current time is %d\n", timer);

    for(i = 0; i < PROCESS_NUM; i++)
    {
      printf("Process %d: arrive time %d, burst time %d, remaining burst time %d\n", 
              processes[i].pid, processes[i].arrive_t, processes[i].burst_t, processes[i].remain_burst_t); 
      
      if(processes[i].arrive_t == timer)
      {
	printf("----------------------------------------\n");
	printf("Number of process(es) in FIFO before adding: %d\n", fifo_count);
        printf("Arrived Process: %d\n", processes[i].pid);
	processes[i].wait_t -= 1;
	// Add the arrived process into the list
	appendToList(processes[i], fifo, fifo_count);
	fifo_count++;
	printf("FIFO added process %d\n", fifo[fifo_count - 1].pid);
	printf("Number of process(es) in FIFO after adding: %d\n", fifo_count);
	printf("Index %d has process %d with remaining burst time %d\n", 
		fifo_count - 1, fifo[fifo_count - 1].pid, fifo[fifo_count - 1].remain_burst_t);
	printf("----------------------------------------\n");
	fifo_checker = 1; // List has process(es), set the flag to 1
	fifo[fifo_count - 1].just_arrived = 1; // Indicates the process has just arrived
      }
    }

    // Do if the list is not empty
    if(fifo_checker)
    {
      printf("FIFO is not empty\n");

      printf("First place of list is process %d\n", fifo[0].pid);
      // Do if the remaining burst time of the first process in the list is larger than 0
      if(fifo[0].remain_burst_t > 0)
      {
	if(fifo[0].execute_t < rr_params.quantum_t && !fifo[0].just_arrived)
	{
	  printf("Before execution: process %d has %d remaining burst time and %d execute time\n", 
		  fifo[0].pid, fifo[0].remain_burst_t, fifo[0].execute_t);
	  fifo[0].remain_burst_t--; // Decrease the remaining burst time
	  fifo[0].execute_t++; // Increase the process execution time
	  printf("After execution: process %d has %d remaining burst time and %d execute time\n", 
		  fifo[0].pid, fifo[0].remain_burst_t, fifo[0].execute_t);
	  // Check if the process execution time equals the quantum time
	  if(fifo[0].execute_t == rr_params.quantum_t && fifo[0].remain_burst_t > 0)
	  {
	    printf("Process %d has reached quantum time with remaining burst time %d, go around!\n", 
		    fifo[0].pid, fifo[0].remain_burst_t);
	    fifo[0].execute_t = 0; // Reset the process execution time
	    // Shift the next ready process to the first palce of the list
	    shiftToLeft(fifo);
	    fifo[0].wait_t += 1;
	    printf("Process %d waiting time before its execution is %d\n", fifo[0].pid, fifo[0].wait_t);
	    printf("First place of list after shift is process %d\n", fifo[0].pid);
	  }
	  // If the process has no remaining burst time left
	  else if(fifo[0].remain_burst_t == 0)
          {
            printf("Process %d has finished its burst time!\n", fifo[0].pid);
	    // Store the completed process into the completed list
	    appendToList(fifo[0], completedList, completed_count);
	    completed_count++;
	    // Delete the completed process from the list
	    deleteFromList(fifo);
	    for(i = 0; i < completed_count; i++)
	    {
	      printf("Completed list has process %d\n", completedList[i].pid);
	    }
          }
        }
      }

      // All other processes would wait when the first process of the list is executing
      for(i = 1; i < fifo_count; i++)
      {
        fifo[i].wait_t++;
        printf("Process %d is on hold with waiting time %d\n", fifo[i].pid, fifo[i].wait_t);
      }
    }
    // Do if no process stored in the list
    else
    {
      printf("List is empty\n");
    }

    fifo[0].just_arrived = 0; // Set the just arrived flag of the process back to 0

    loop_count++; // Keep tracking the number of loops
    printf("The %d loop completed!\n\n", loop_count); 
  }
}

// Calculate the average waiting time of the Round Robin CPU Scheduling
float getAverageWaitingTime(Process fifo[])
{
  float total_wait_t, avg_wait_t;

  for(i = 0; i < PROCESS_NUM; i++)
  {
    total_wait_t += fifo[i].wait_t;
  }
  avg_wait_t = total_wait_t / PROCESS_NUM;
  
  return avg_wait_t;
}

// Calculate the average turnaround time of the Round Robin CPU Scheduling
float getAverageTurnaroundTime(Process fifo[])
{
  float total_wait_t, total_turnaround_t, avg_turnaround_t;
  
  for(i = 0; i < PROCESS_NUM; i++)
  {
    total_wait_t += fifo[i].wait_t;
  }
  total_turnaround_t = total_wait_t + processes->total_burst_t;
  avg_turnaround_t = total_turnaround_t / PROCESS_NUM;

  return avg_turnaround_t;
}

// Append the process to the end of the list
void appendToList(Process process, Process fifo[], int index)
{
  fifo[index] = process;
  index++;
}

// Shift the process to the left which is the first place of the list
void shiftToLeft(Process fifo[])
{
  Process temp;
  temp = fifo[0];

  for(i = 0; i < fifo_count; i++)
  {
    fifo[i] = fifo[i + 1];
  }

  fifo[fifo_count - 1] = temp;  
}

// Delete the process from the list
void deleteFromList(Process fifo[])
{
  for(i = 0; i < fifo_count - 1; i++)
  {
    fifo[i] = fifo[i + 1];
  }

  fifo_count--;
}

// Initialize the processes at the beginning
void initializeProcesses()
{
  processes[0].pid = 1; processes[0].arrive_t = 8;  processes[0].burst_t = 10;
  processes[1].pid = 2; processes[1].arrive_t = 10; processes[1].burst_t = 3;
  processes[2].pid = 3; processes[2].arrive_t = 14; processes[2].burst_t = 7;
  processes[3].pid = 4; processes[3].arrive_t = 9;  processes[3].burst_t = 5;
  processes[4].pid = 5; processes[4].arrive_t = 16; processes[4].burst_t = 4;
  processes[5].pid = 6; processes[5].arrive_t = 21; processes[5].burst_t = 6;
  processes[6].pid = 7; processes[6].arrive_t = 26; processes[6].burst_t = 2;
	
  /* 
   * Set the remaining burst time same as the burst time at the beginning
   * Calculate the total burst time needed for all processes
   * Initial the process execute time to 0
   */
  for (i = 0; i < PROCESS_NUM; i++) 
  {
    processes[i].remain_burst_t = processes[i].burst_t;
    processes->total_burst_t += processes[i].burst_t;
    processes[i].execute_t = 0;
  }
}

// Write and send the average wait time and the average turnaround time to FIFO
void send_FIFO() 
{
  int res, fifofd;
  char *myfifo = "myfifo.txt";

  // Create the named pipe FIFO
  res = mkfifo(myfifo, 0777);
  // Check the named pipe creation
  if(res < 0) 
  {
    printf("mkfifo error\n");
    exit(0);
  }

  // Single the sem_RR semaphore
  sem_post(&sem_RR);

  // Open the fifofd for write only
  fifofd = open(myfifo, O_WRONLY);
  // Check the validation of fifofd
  if(fifofd < 0) 
  {
    printf("fifo open send error\n");
    exit(0);
  }

  // Write the average waiting time and average turnaround time into fifofd
  write(fifofd, &avg_wait_t, sizeof(avg_wait_t));
  write(fifofd, &avg_turnaround_t, sizeof(avg_turnaround_t));

  // Close the fifofd
  close(fifofd);
  // Unlink fifo name
  unlink(myfifo);
}

/* 
 * Read the average waiting time and the average turnaround time from fifo 
 * then write to output.txt
 */
void read_FIFO() 
{
  int fifofd;
  float fifo_avg_turnaround_t, fifo_avg_wait_t;
  char *myfifo = "myfifo.txt";
  static const char file[] = "output.txt"; // output.txt
  
  // Open the file to write
  FILE *writer = fopen(file, "w");
  // Check the file validation
  if(!writer)
  {
    perror(file);
    exit(0);
  }  

  // Open the fifofd for read only
  fifofd = open(myfifo, O_RDONLY);
  // Check the validation of fifofd
  if(fifofd < 0) 
  {
    printf("fifo open read error\n");
    exit(0);
  }

  // Read the average waiting time and average turnaround time from fifofd
  read(fifofd, &fifo_avg_wait_t, sizeof(int));
  read(fifofd, &fifo_avg_turnaround_t, sizeof(int));
	
  printf("\nRead from FIFO: %fs Average waiting time\n", fifo_avg_wait_t);
  printf("Read from FIFO: %fs Average turnaround time\n", fifo_avg_turnaround_t);

  // Put the average waiting and turnaround time into the file output.txt
  fputs("Average waiting time is ", writer);
  fprintf(writer, "%f.\n", fifo_avg_wait_t);
  fputs("Average turnaround time is ", writer);
  fprintf(writer, "%f.\n", fifo_avg_turnaround_t);

  fclose(writer); // Close the file
  printf("\nFinished writing data to output.txt!\n");
  close(fifofd); // Close the fifofd
  remove(myfifo); // remove the fifofd
}
