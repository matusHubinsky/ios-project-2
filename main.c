
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#define ARG_NUM 6


int nz;		// pocet zakaznikov
int nu;		// pocet uradnikov  
int tz;		// maximalny cas co caka zakaznik od vytvorenia po prichod na postu
int tu;		// maximalna dlzka prestavky uradnika 
int f;		// maximálny čas v milisekundách 


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
	
	// fprintf(stdout, "%d %d %d %d %d\n", nz, nu, tz, tu, f);

	if (nz < 1 || nu < 1) {
		fprintf(stderr, "Error: Wrong number of people!\n");
		return 1;
	}

	if (tz < 0 || tu < 0 || f < 0) {
		fprintf(stderr, "Error: Wrong waiting time!\n");
		return 1;	
	}

	return 0;
}