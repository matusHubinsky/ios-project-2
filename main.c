
#include <math.h>
#include <time.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <semaphore.h>

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
struct queue_t* letter_queue = NULL;
struct queue_t* package_queue = NULL;
struct queue_t* money_queue = NULL;

static void semaphores_open_all() {
    if ((xhubin04_semaphore_customer = sem_open("/xhubin04_semaphore_customer", O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED) { 
        fprintf(stderr, "sem_open: xhubin04_semaphore_customer");
        exit(1);
    }

    if ((xhubin04_semaphore_official = sem_open("/xhubin04_semaphore_official", O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED) {
        fprintf(stderr, "sem_open: xhubin04_semaphore_official");
        exit(1);
    }

    if ((xhubin04_semaphore_customer_done = sem_open("/xhubin04_semaphore_customer_done", O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED) { 
        fprintf(stderr, "sem_open: xhubin04_semaphore_customer_done");
        exit(1);
    }

	if ((xhubin04_semaphore_official_done = sem_open("/xhubin04_semaphore_official_done", O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_official_done");
        exit(1);
    }

	if ((xhubin04_semaphore_mutex = sem_open("/xhubin04_semaphore_mutex", O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_mutex");
        exit(1);
    }

	if ((xhubin04_semaphore_write = sem_open("/xhubin04_semaphore_write", O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED){
        fprintf(stderr, "sem_open: xhubin04_semaphore_write");
        exit(1);
    }

}


static void semaphores_close_all() {
	if (sem_close(xhubin04_semaphore_official) == -1) {
        fprintf(stderr, "sem_close: xhubin04_semaphore_lette");
        exit(1);
    }

	if (sem_close(xhubin04_semaphore_customer_done) == -1) {
        fprintf(stderr, "sem_close: xhubin04_semaphore_customer_done");
        exit(1);
    }

	if (sem_close(xhubin04_semaphore_customer) == -1) {
        fprintf(stderr, "sem_close: xhubin04_semaphore_customer");
        exit(1);
    }

	if (sem_close(xhubin04_semaphore_official_done) == -1) {
        fprintf(stderr, "sem_close: xhubin04_semaphore_official_done");
        exit(1);
    }

	if (sem_close(xhubin04_semaphore_mutex) == -1) {
        fprintf(stderr, "sem_close: xhubin04_semaphore_mutex");
        exit(1);
    }

	if (sem_close(xhubin04_semaphore_write) == -1) {
        fprintf(stderr, "sem_close: xhubin04_semaphore_write");
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


void mmap_init() {
	line_number = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	customer_number = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	official_number = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	letter_queue = queue_init();
	package_queue = queue_init();
	money_queue = queue_init();
}


void nmap_unlink() {
	munmap(line_number, sizeof(int));
	munmap(customer_number, sizeof(int));
	munmap(official_number, sizeof(int));
	queue_destroy(letter_queue);
	queue_destroy(package_queue);
	queue_destroy(money_queue);
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
	// file buffer
    setbuf(output, NULL);

	// open all semaphores
	semaphores_open_all();
	
	// close all semaphores
	semaphores_close_all();
	
	// unlink all semaphores
	semaphores_unlink_all();

	fclose(output);

	return 0;
}