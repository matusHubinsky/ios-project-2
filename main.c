
#include <math.h>
#include <time.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <semaphore.h>
#include <string.h>

#include <sys/mman.h>          
#include <sys/stat.h>
#include <sys/wait.h>

#include "queue.h"

#define ARG_NUM 6


int nz;	// number of customers
int nu;	// number of officials
int tz;	// the maximum waiting time that customers waits
int tu;	// the maximum length of the official's break time
int f;	// the maximum time in miliseconds

FILE* output;

sem_t* xhubin04_semaphore_customer = NULL;
sem_t* xhubin04_semaphore_official = NULL;
sem_t* xhubin04_semaphore_customer_done = NULL;
sem_t* xhubin04_semaphore_official_done = NULL;
sem_t* xhubin04_semaphore_mutex = NULL;
sem_t* xhubin04_semaphore_write = NULL;

int* line_number = 0;
int* customer_number = 0;
int* official_number = 0;
bool* post_office = false;
struct queue_t* letter_queue = NULL;
struct queue_t* package_queue = NULL;
struct queue_t* money_queue = NULL;


static void semaphores_open_all() {
    if ((xhubin04_semaphore_customer = sem_open("/xhubin04_semaphore_customer", O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED) { 
        fprintf(stderr, "sem_open: xhubin04_semaphore_customer");
        exit(1);
    }

    if ((xhubin04_semaphore_official = sem_open("/xhubin04_semaphore_official", O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED) {
        fprintf(stderr, "sem_open: xhubin04_semaphore_official\n");
        exit(1);
    }

    if ((xhubin04_semaphore_customer_done = sem_open("/xhubin04_semaphore_customer_done", O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED) { 
        fprintf(stderr, "sem_open: xhubin04_semaphore_customer_done\n");
        exit(1);
    }

	if ((xhubin04_semaphore_official_done = sem_open("/xhubin04_semaphore_official_done", O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_official_done\n");
        exit(1);
    }

	if ((xhubin04_semaphore_mutex = sem_open("/xhubin04_semaphore_mutex", O_CREAT|O_EXCL, 0666, 1)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_mutex\n");
        exit(1);
    }

	if ((xhubin04_semaphore_write = sem_open("/xhubin04_semaphore_write", O_CREAT|O_EXCL, 0666, 1)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_write\n");
        exit(1);
    }

}


static void semaphores_close_all() {
	if (sem_close(xhubin04_semaphore_official) == -1) {
        fprintf(stderr, "sem_close: xhubin04_semaphore_letter\n");
        exit(1);
    }

	if (sem_close(xhubin04_semaphore_customer_done) == -1) {
        fprintf(stderr, "sem_close: xhubin04_semaphore_customer_done\n");
        exit(1);
    }

	if (sem_close(xhubin04_semaphore_customer) == -1) {
        fprintf(stderr, "sem_close: xhubin04_semaphore_customer\n");
        exit(1);
    }

	if (sem_close(xhubin04_semaphore_official_done) == -1) {
        fprintf(stderr, "sem_close: xhubin04_semaphore_official_done\n");
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
	sem_unlink("/xhubin04_semaphore_customer");
	sem_unlink("/xhubin04_semaphore_official");
	sem_unlink("/xhubin04_semaphore_customer_done");
	sem_unlink("/xhubin04_semaphore_official_done");
	sem_unlink("/xhubin04_semaphore_mutex");
	sem_unlink("/xhubin04_semaphore_write");
}


static void mmap_init() {
	line_number = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	customer_number = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	official_number = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	post_office = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	letter_queue = mmap(NULL, sizeof(struct queue_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	package_queue = mmap(NULL, sizeof(struct queue_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	money_queue = mmap(NULL, sizeof(struct queue_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}


static void mmap_unlink() {
	munmap(line_number, sizeof(int));
	munmap(customer_number, sizeof(int));
	munmap(official_number, sizeof(int));
	munmap(post_office, sizeof(bool));
	munmap(letter_queue, sizeof(struct queue_t));
	munmap(package_queue, sizeof(struct queue_t));
	munmap(money_queue, sizeof(struct queue_t));
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

	sem_t* xhubin04_semaphore_self;
	char sem_name[100] = "xhubin04_semaphore_self_";
	char number_string[20];
	snprintf(number_string, sizeof(number_string), "%d", id);
    strcat(sem_name, number_string);

	if ((xhubin04_semaphore_self = sem_open(sem_name, O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED) { 
        fprintf(stderr, "error sem_open: %s\n", sem_name);
        exit(1);
    }

	if (*post_office) {
		sem_wait(xhubin04_semaphore_mutex);

		int queue_number = rand() % 3 + 1;
		write_to_file("Z %d: entering office for a service %d\n", id, queue_number);

		if (queue_number == 1) {
			queue_push(letter_queue, xhubin04_semaphore_self);
		} else if (queue_number == 2) {
			queue_push(package_queue, xhubin04_semaphore_self);
		} else if (queue_number == 3) {
			queue_push(money_queue, xhubin04_semaphore_self);
		}

		sem_post(xhubin04_semaphore_mutex);
		sem_post(xhubin04_semaphore_customer);

		sem_wait(xhubin04_semaphore_official_done);

		write_to_file("Z %d: called by office_worker\n", id);
	
		//
		usleep((rand() % 10) * 1000);
		sem_post(xhubin04_semaphore_mutex);
		sem_wait(xhubin04_semaphore_self);
	}
		
	write_to_file("Z %d: going home\n", id);

	if (sem_close(xhubin04_semaphore_self) == -1) {
        fprintf(stderr, "sem_close: %s\n", sem_name);
        exit(1);
    }

	sem_unlink(sem_name);
	exit(0);
}



void official(int id) {
	srand(time(0) ^ getpid());
	write_to_file("U %d: started\n", id);

	usleep((rand() % (tu + 1)) * 1000);

	while (true) {
		sem_wait(xhubin04_semaphore_customer);
		if (is_empty(letter_queue) && is_empty(package_queue) && is_empty(money_queue)) {
			sem_wait(xhubin04_semaphore_mutex);

			struct queue_t* working_queue = NULL;
			int pick = -1;
			do {
				pick = rand() % 3 + 1;
				if (pick == 1) {
					working_queue = letter_queue;
				} else if (pick == 2) {
					working_queue = package_queue;
				} else if (pick == 3) {
					working_queue = money_queue;
				}
			} while (!is_empty(working_queue));

			sem_post(xhubin04_semaphore_mutex);

			write_to_file("U %d: serving a service of type %d\n", id, pick);
			
			sem_t* sem = queue_pop(working_queue);
			sem_post(sem);

			usleep((rand() % 10) * 1000);
			
			write_to_file("U %d: service finished\n");

			sem_wait(xhubin04_semaphore_customer_done);
			sem_post(xhubin04_semaphore_official_done);
			
		} else if (*post_office) {
			write_to_file("U %d: taking brake\n", id);
			usleep((rand() % (tu + 1)) * 1000);
			write_to_file( "U %d: break finished\n", id);
		} else {
			write_to_file("U %d: going home\n", id);
			sem_post(xhubin04_semaphore_customer);
		}
		// sem_post(xhubin04_semaphore_customer);
	}

	// 
	exit(0);
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
	if (nz < 0 || nu < 0) {
		fprintf(stderr, "Error: Wrong number of people!\n");
		return 1;
	}

	// check if waiting times are correct, it must be bigger than 0 and lower than 1000
	if ((tz < 0 || tz > 1000) || (tu < 0 || tu > 1000) || (f < 0 || f > 1000)) {
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
	
	// 
	mmap_init();
	letter_queue = queue_init();
	package_queue = queue_init();
	money_queue = queue_init();

	*post_office = true;

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

	usleep((f/2 + (rand() % f/2)) * 1000);
	write_to_file("closing\n");
	*post_office = false;
	while(wait(NULL) > 0);
	


	write_to_file("last\n");
	// close all semaphores
	semaphores_close_all();
	
	// unlink all semaphores
	semaphores_unlink_all();

	queue_destroy(letter_queue);
	queue_destroy(package_queue);
	queue_destroy(money_queue);
	// unmap all memory
	mmap_unlink();

	fclose(output);

	return 0;
}