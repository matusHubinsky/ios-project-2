
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

FILE* output;

sem_t* xhubin04_semaphore_letter = NULL;
sem_t* xhubin04_semaphore_package = NULL;
sem_t* xhubin04_semaphore_money = NULL;
sem_t* xhubin04_semaphore_mutex = NULL;
sem_t* xhubin04_semaphore_write = NULL;

int* queue_letter = NULL;
int* queue_package = NULL;
int* queue_money = NULL;
int* line_number = NULL;
bool* post_office = NULL;


static void semaphores_open_all() {
	if ((xhubin04_semaphore_letter = sem_open("xhubin04_semaphore_letter", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_letter\n");
        exit(1);
    }

	if ((xhubin04_semaphore_package = sem_open("/xhubin04_semaphore_package", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_package\n");
        exit(1);
    }

	if ((xhubin04_semaphore_money = sem_open("/xhubin04_semaphore_money", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_money\n");
        exit(1);
    }

	if ((xhubin04_semaphore_mutex = sem_open("/xhubin04_semaphore_mutex", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_mutex\n");
        exit(1);
    }

	if ((xhubin04_semaphore_write = sem_open("/xhubin04_semaphore_write", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_write\n");
        exit(1);
    }
}


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


static void semaphores_unlink_all() {
	sem_unlink("xhubin04_semaphore_letter");
	sem_unlink("xhubin04_semaphore_package");
	sem_unlink("xhubin04_semaphore_money");
	sem_unlink("/xhubin04_semaphore_mutex");
	sem_unlink("/xhubin04_semaphore_write");
}


static void mmap_init() {
	queue_letter = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	queue_package = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	queue_money = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	line_number = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	post_office = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}


static void mmap_unlink() {
	munmap(queue_letter, sizeof(int));
	munmap(queue_package, sizeof(int));
	munmap(queue_money, sizeof(int));
	munmap(line_number, sizeof(int));
	munmap(post_office, sizeof(bool));
}


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


void customer(int id) {
	srand(time(0) ^ getpid());
	
	write_to_file("Z %d: started\n", id);
	usleep((rand() % (tz + 1)) * 1000);

	if (*post_office) {	
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


bool customers_in_queue() {
	// fprintf(stderr, "%d %d %d\n", (*queue_letter), (*queue_package), (*queue_money));
	// fflush(stderr);
	if ((*queue_letter) || (*queue_package) || (*queue_money)) {
		return true;
	}
	return false; 
}


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
	else if (*post_office) {
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


bool is_number(char *ptr) {
	long long num;
	num = strtol(ptr, &ptr, 10);
	if (*ptr == '\0') {
		num++;
		return true;
	}
	else {
		num++;
		return false;
	}
}


int main(int argc, char *argv[]) {
	// check number of arguments
	if (argc != ARG_NUM) {
		fprintf(stderr, "Error: Wrong number of argments!\n");
		return 1;
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
		return 1;
	}

	if (!is_number(argv[1]) || !is_number(argv[2]) || !is_number(argv[3]) || !is_number(argv[4]) || !is_number(argv[5])) {
		fprintf(stderr, "Error: Arguments are not numbers!\n");
		return 1;	
	}

	// check if waiting times are correct, it must be bigger than 0 and lower than 1000
	if ((tz < 0 || tz > 10000) || (tu < 0 || tu > 100) || (f < 0 || f > 1000)) {
		fprintf(stderr, "Error: Wrong waiting time!\n");
		return 1;	
	}

	output = fopen("proj2.out", "w");
	if (output == NULL) {
     	fprintf(stderr, "File can't be created\n");
    	exit(1);
    }

	// open all semaphores
	semaphores_open_all();
	
	// s
	mmap_init();
	
	memory_lock(*line_number = 1);
	memory_lock(*post_office = true);

	for (int i = 0; i < nz; i++) {
		pid_t customer_pid = fork();
		if (customer_pid == 0) {
			customer(i + 1);
			return 0;
		} else if (customer_pid == -1) {
			fprintf(stderr, "Error: fork\n");
			return 1;
		}
	}

	for (int i = 0; i < nu; i++) {
		pid_t official_pid = fork();
		if (official_pid == 0) {
			official(i + 1);
			return 0;
		} else if (official_pid == -1) {
			fprintf(stderr, "Error: fork\n");
			return 1;
		}
	}

	usleep(((f/2) + (rand() % (f/2 + 1))) * 1000);
	write_to_file("closing\n");
	memory_lock(*post_office = false);
	
	// close all children
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