/*
 * @author Matus Hubinsky xhubin04
 * @date 26.04.2022
 * @brief program for the post office problem inspired by The Barbershop problem from "The Little Book of Semaphores"
 * @file main.c
*/


#include <math.h>
#include <time.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <signal.h>
#include <semaphore.h>
#include <string.h>
#include <sys/mman.h>          
#include <sys/stat.h>
#include <sys/wait.h>

#include "main.h"


int nz;	// number of customers
int nu;	// number of officials
int tz;	// the maximum waiting time that customers waits
int tu;	// the maximum length of the official's break time
int f;	// the maximum time in miliseconds

FILE* output; // output file

sem_t* xhubin04_semaphore_letter = NULL;  	// letter queue semaphore
sem_t* xhubin04_semaphore_package = NULL; 	// package queue semaphore
sem_t* xhubin04_semaphore_money = NULL;		// money queue semaphore
sem_t* xhubin04_semaphore_mutex = NULL;		// mutex semaphore
sem_t* xhubin04_semaphore_write = NULL;		// semaphore for writing to output file

int* queue_letter = NULL;	// number of customers in letter queue
int* queue_package = NULL;	// number of customers in package queue
int* queue_money = NULL;	// number of customers in money queue
int* line_number = NULL;	// number of current line
bool* post_office = NULL;	// post office status (true - open, false - closed)


/*
 * @brief open all semaphores 
 * @param
 * @returns
*/
static void semaphores_open_all() {
	if ((xhubin04_semaphore_letter = sem_open("xhubin04_semaphore_letter", O_CREAT | O_EXCL, 0664, 0)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_letter\n");
        exit(1);
    }

	if ((xhubin04_semaphore_package = sem_open("/xhubin04_semaphore_package", O_CREAT | O_EXCL, 0664, 0)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_package\n");
        exit(1);
    }

	if ((xhubin04_semaphore_money = sem_open("/xhubin04_semaphore_money", O_CREAT | O_EXCL, 0664, 0)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_money\n");
        exit(1);
    }

	if ((xhubin04_semaphore_mutex = sem_open("/xhubin04_semaphore_mutex", O_CREAT | O_EXCL, 0664, 1)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_mutex\n");
        exit(1);
    }

	if ((xhubin04_semaphore_write = sem_open("/xhubin04_semaphore_write", O_CREAT | O_EXCL, 0664, 1)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_write\n");
        exit(1);
    }
}


/*
 * @brief close all semaphores 
 * @param
 * @return
*/
static void semaphores_close_all() {
	if (sem_close(xhubin04_semaphore_letter) == -1) {
        fprintf(stderr, "sem_open: xhubin04_semaphore_letter\n");
        exit(1);
    }

	if (sem_close(xhubin04_semaphore_package) == -1) {
        fprintf(stderr, "sem_open: xhubin04_semaphore_package\n");
        exit(1);
    }

	if (sem_close(xhubin04_semaphore_money) == -1) {
        fprintf(stderr, "sem_open: xhubin04_semaphore_money\n");
        exit(1);
    }

	if (sem_close(xhubin04_semaphore_mutex) == -1) {
        fprintf(stderr, "sem_close: xhubin04_semaphore_mutex\n");
        exit(1);
    }

	if (sem_close(xhubin04_semaphore_write) == -1) {
        fprintf(stderr, "sem_close: xhubin04_semaphore_write\n");
        exit(1);
    }
}


/*
 * @brief unlink all semaphores 
 * @param
 * @return
*/
static void semaphores_unlink_all() {
	sem_unlink("xhubin04_semaphore_letter");
	sem_unlink("xhubin04_semaphore_package");
	sem_unlink("xhubin04_semaphore_money");
	sem_unlink("/xhubin04_semaphore_mutex");
	sem_unlink("/xhubin04_semaphore_write");
}


/*
 * @brief initialize shared memory 
 * @param
 * @return
*/
static void mmap_init() {
	queue_letter = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	queue_package = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	queue_money = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	line_number = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	post_office = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}


/*
 * @brief unlink shared memory 
 * @param
 * @return
*/
static void mmap_unlink() {
	munmap(queue_letter, sizeof(int));
	munmap(queue_package, sizeof(int));
	munmap(queue_money, sizeof(int));
	munmap(line_number, sizeof(int));
	munmap(post_office, sizeof(bool));
}


/*
 * @brief close semaphore for writing, write message and open semaphore
 * @param format string, same as printf
 * @return
*/
void write_to_file(char *format, ...) {
	sem_wait(xhubin04_semaphore_write);

	va_list args;
   	va_start(args, format);
	fprintf(output, "%d: ", (*line_number)++);
   	vfprintf(output, format, args);
	fflush(output);
   	va_end(args);

	sem_post(xhubin04_semaphore_write);
}


/*
 * @brief check if the is any customer in queues, lock memory while checking state
 * @paran 
 * @return true if the are customers in queues, false if not
*/
bool customers_in_queue() {
	bool result = false;
	sem_wait(xhubin04_semaphore_mutex);
	if ((*queue_letter) || (*queue_package) || (*queue_money)) {
		result = true;
	} else {
		result = false;
	}
	sem_post(xhubin04_semaphore_mutex);
	return result;
}


/*
 * @brief check if post office is open, lock memory while checking state
 * @paran 
 * @return true if post office is open, false if not
*/
bool check_office() {
	bool result = false;
	sem_wait(xhubin04_semaphore_mutex);
	result = *post_office;
	sem_post(xhubin04_semaphore_mutex);
	return result;
}


/*
 * @brief process customer 
 * @param id customer's id
 * @return
*/
void customer(int id) {
	srand(time(0) ^ getpid());
	
	write_to_file("Z %d: started\n", id);
	usleep((rand() % (tz + 1)) * 1000);

	if (check_office()) {	
		int queue_number = rand() % 3 + 1;
		write_to_file("Z %d: entering office for a service %d\n", id, queue_number);

		if (queue_number == 1) {
			memory_lock((*queue_letter)++);
			sem_wait(xhubin04_semaphore_letter);
		} else if (queue_number == 2) {
			memory_lock((*queue_package)++);
			sem_wait(xhubin04_semaphore_package);	
		} else if (queue_number == 3) {
			memory_lock((*queue_money)++);
			sem_wait(xhubin04_semaphore_money);
		}

		write_to_file("Z %d: called by office worker\n", id);
		usleep((rand() % 11) * 1000);
	}
	write_to_file("Z %d: going home\n", id);
}


/*
 * @brief process official 
 * @param id official's id
 * @return
*/
void official(int id) {
	srand(time(0) ^ getpid());
	write_to_file("U %d: started\n", id);

	start:
	if (customers_in_queue()) {	
		int pick = -1;
		int value = 0;
		sem_wait(xhubin04_semaphore_mutex);

		do {
			pick = rand() % 3 + 1;
			if (pick == 1) {
				value = *queue_letter;
			} else if (pick == 2) {
				value = *queue_package;
			} else {
				value = *queue_money;
			}
		} while (value <= 0);

		if (pick == 1) {
			(*queue_letter)--;
			sem_post(xhubin04_semaphore_letter);
		} else if (pick == 2) {
			(*queue_package)--;
			sem_post(xhubin04_semaphore_package);
		} else {
			(*queue_money)--;
			sem_post(xhubin04_semaphore_money);
		}
	
		sem_post(xhubin04_semaphore_mutex);

		write_to_file("U %d: serving a service of type %d\n", id, pick);

		usleep((rand() % 10) * 1000);
		write_to_file("U %d: service finished\n");
		goto start;
	} 
	else if (check_office()) {
		write_to_file("U %d: taking break\n", id);
		usleep((rand() % (tu + 1)) * 1000);
		write_to_file( "U %d: break finished\n", id);
		goto start;
	} 
	else {
		write_to_file("U %d: going home\n", id);
	}

	exit(0);
}


/*
 * @brief check if string is number 
 * @param string pointer to string
 * @return true when the string was a number, false otherwise
*/
bool is_number(char *string) {
	int number;
	number = strtol(string, &string, 10);
	if (*string == '\0') {
		number++;
		return true;
	}
	else {
		number++;
		return false;
	}
}


/*
 * @brief check all argumetns, assign values to variables, check if arguments are correct
 * print error message and exit with code 1 if arguments are not correct
 * @param argc nubmer of arguments
 * @param argv string array of arguments
 * @return
*/
void process_arguments(int argc, char *argv[]) {
	// check number of arguments
	if (argc != ARG_NUM) {
		fprintf(stderr, "Error: Wrong number of argments!\n");
		exit(1);
	}

	// assing arguments values
	nz = atoi(argv[1]); 	
	nu = atoi(argv[2]); 	
	tz = atoi(argv[3]); 	
	tu = atoi(argv[4]); 	
	f = atoi(argv[5]); 	

	// check if arguments are correct, they must be bigger than 0
	if (nz < -1 || nu < 1) {
		fprintf(stderr, "Error: Wrong number of people!\n");
		exit(1);
	}

	// check if arguments are numbers and not strings
	if (!is_number(argv[1]) || !is_number(argv[2]) || !is_number(argv[3]) || !is_number(argv[4]) || !is_number(argv[5])) {
		fprintf(stderr, "Error: Arguments are not numbers!\n");
		exit(1);	
	}

	// check if waiting times are correct, it must be bigger than 0 and lower than 1000
	if ((tz < 0 || tz > 10000) || (tu < 0 || tu > 100) || (f < 0 || f > 1000)) {
		fprintf(stderr, "Error: Wrong waiting time!\n");
		exit(1);	
	}
}


int main(int argc, char *argv[]) {
	// process all arguments and assign values to variables
	process_arguments(argc, argv);

	output = fopen("proj2.out", "w");
	if (output == NULL) {
     	fprintf(stderr, "File can't be created\n");
    	exit(1);
    }

	// open all semaphores
	semaphores_open_all();
	
	// map all shared variables
	mmap_init();
	
	// initialize all shared variables
	memory_lock(*line_number = 1);
	memory_lock(*post_office = true);

	// fork all customers
	for (int i = 0; i < nz; i++) {
		pid_t customer_pid = fork();
		if (customer_pid == 0) {
			customer(i + 1);
			return 0;
		} else if (customer_pid == -1) {
			fprintf(stderr, "Error: can't fork customer\n");
			return 1;
		}
	}

	// fork all officials
	for (int i = 0; i < nu; i++) {
		pid_t official_pid = fork();
		if (official_pid == 0) {
			official(i + 1);
			return 0;
		} else if (official_pid == -1) {
			fprintf(stderr, "Error: can't fork official\n");
			return 1;
		}
	}

	// go for sleep for random nubmer in range <f/2, f>, then close the post office
	usleep(((f/2) + (rand() % (f/2 + 1))) * 1000);
	write_to_file("closing\n");
	memory_lock(*post_office = false);
	
	// wait for all children to die
	while(wait(NULL) > 0);
	
	// close all semaphores
	semaphores_close_all();
	
	// unlink all semaphores
	semaphores_unlink_all();

	// unmap all memory
	mmap_unlink();

	fclose(output);

	return 0;
}