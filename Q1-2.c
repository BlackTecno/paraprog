#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/times.h>
#include <limits.h>
#include <unistd.h>

void *find_min(void *list_ptr);
unsigned int time_seed();
double get_seconds();


pthread_mutex_t minimum_value_lock;
int minimum_value, partial_list_size;
int MIN_INT = INT_MAX;
int SIZE = 100000000;


void main() {

	double start, end, finaltime;
	int i, x, threadsize;
	unsigned int seed = (unsigned int)time(NULL);
	minimum_value = MIN_INT;
	partial_list_size = SIZE;
	pthread_mutex_init(&minimum_value_lock, NULL);
	int *listm = (int*)malloc(sizeof(int)*SIZE);
	//      pthread_t thread[8];
	threadsize = 1;
	/* RANDOMIZE THE LIST */


	for (i = 0; i < SIZE; i++) {
		listm[i] = rand_r(&seed) % 10000;

	}

	/* FOR LOOP WILL TEST EACH THREAD SIZE OF 1, 2, 4, AND 8
	IN A SINGLE EXECUTION */

	for (x = 0; x < 4; x++) {
		minimum_value = INT_MAX;

		switch (x) {

		case 0:
			threadsize = 1;
			break;
		case 1:
			threadsize = 2;
			break;
		case 2:
			threadsize = 4;
			break;
		case 3:
			threadsize = 8;
			break;
		default:                                //for safety
			break;
		}


		pthread_t thread[threadsize];
		partial_list_size = SIZE / threadsize;


		/* START MIN SEARCH AND TIMER */

		start = get_seconds();

		for (i = 0; i < threadsize; i++) {
			pthread_create(&thread[i], NULL, find_min, (void*)&listm[i*partial_list_size]);
		}

		/* JOIN THREADS */

		for (i = 0; i < threadsize; i++) {
			pthread_join(thread[i], NULL);
		}

		end = get_seconds();
		finaltime = end - start;

		printf("Time for %d threads is %f.\n", threadsize, finaltime);
		printf("Minimum value found was %d. \n", minimum_value);
	}
}

void *find_min(void *list_pnt) {
	int *partial_list_pointer, my_min, i;
	my_min = MIN_INT;
	partial_list_pointer = (int *)list_pnt;

	for (i = 0; i < partial_list_size; i++) {
		if (partial_list_pointer[i] < my_min)
			my_min = partial_list_pointer[i];
	}

	//      printf("min of list is: %d\n", my_min);         //FOR TESTING

			/* lock the mutex associated with minimum_value
					and update the variable as required */

	pthread_mutex_lock(&minimum_value_lock);
	if (my_min < minimum_value)
		minimum_value = my_min;

	/* and unlock the mutex */

	pthread_mutex_unlock(&minimum_value_lock);
	pthread_exit(0);
}

double get_seconds() {
	struct timeval tvs;
	//      struct timezone tzs;

	gettimeofday(&tvs, NULL);

	return((double)tvs.tv_sec + (double)(tvs.tv_usec * 1.e-6));
}

