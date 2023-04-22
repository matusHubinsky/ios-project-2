
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


#define ARG_NUM 6


int nz;		// pocet zakaznikov
int nu;		// pocet uradnikov  
int tz;		// maximalny cas co caka zakaznik od vytvorenia po prichod na postu
int tu;		// maximalna dlzka prestavky uradnika 
int f;		// maximálny čas v milisekundách 

FILE* file_ptr;

sem_t *xhubin04_semaphore_customer = NULL;
sem_t *xhubin04_semaphore_official = NULL;
sem_t *xhubin04_semaphore_customer_done = NULL;
sem_t *xhubin04_semaphore_official_done = NULL;
sem_t *xhubin04_semaphore_mutex = NULL;
sem_t *xhubin04_semaphore_write = NULL;


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


int main(int argc, char *argv[]) {
	if (argc != ARG_NUM) {
		fprintf(stderr, "Error: Wrong number of argments!\n");
		return 1;
	}

	nz = atoi(argv[1]); 	
	nu = atoi(argv[2]); 	
	tz = atoi(argv[3]); 	
	tu = atoi(argv[4]); 	
	f = atoi(argv[5]); 	

	if (nz < 0 || nu < 0) {
		fprintf(stderr, "Error: Wrong number of people!\n");
		return 1;
	}

	if ((tz < 0 || tz > 1000) || (tu < 0 || tu > 1000) || (f < 0 || f > 1000)) {
		fprintf(stderr, "Error: Wrong waiting time!\n");
		return 1;	
	}

	semaphores_open_all();
	semaphores_close_all();
	semaphores_unlink_all();
    
	return 0;
}