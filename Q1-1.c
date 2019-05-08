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

typedef struct {
	int reader;
	int writer;
	pthread_cond_t readerP;
	pthread_cond_t writerP;
	int pendW;
	pthread_mutex_t read_write_lock;
}mylib_rwlock_t;

void mylib_rwlock_init(mylib_rwlock_t *rwstruct);
void mylib_rwlock_rlock(mylib_rwlock_t *rwstruct);
void mylib_rwlock_wlock(mylib_rwlock_t *rwstruct);
void mylib_rwlock_unlock(mylib_rwlock_t *rwstruct);

pthread_mutex_t minimum_value_lock;
int minimum_value, partial_list_size;
mylib_rwlock_t read_write_lock;
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
	mylib_rwlock_init(&read_write_lock);
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
	mylib_rwlock_unlock(&read_write_lock);
	if (my_min < minimum_value) {
		mylib_rwlock_unlock(&read_write_lock);
		mylib_rwlock_wlock(&read_write_lock);
		minimum_value = my_min;
	}

	mylib_rwlock_unlock(&read_write_lock);
	pthread_exit(0);
}

void mylib_rwlock_init(mylib_rwlock_t *rwstruct) {
	rwstruct->reader = rwstruct->writer = rwstruct->pendW = 0;
	pthread_mutex_init(&(rwstruct->read_write_lock), NULL);
	pthread_cond_init(&(rwstruct->readerP), NULL);
	pthread_cond_init(&(rwstruct->writerP), NULL);
}

void mylib_rwlock_rlock(mylib_rwlock_t *rwstruct) {
	pthread_mutex_lock(&(rwstruct->read_write_lock));
	while ((rwstruct->writer > 0) || (rwstruct->reader > 0)) {
		rwstruct->pendW++;
		pthread_cond_wait(&(rwstruct->writerP), &(rwstruct->read_write_lock));
	}

	rwstruct->pendW--;
	rwstruct->writer++;
	pthread_mutex_unlock(&(rwstruct->read_write_lock));
}

void mylib_rwlock_wlock(mylib_rwlock_t *rwstruct) {
	pthread_mutex_lock(&(rwstruct->read_write_lock));
	while ((rwstruct->writer > 0) || (rwstruct->reader > 0)) {
		rwstruct->pendW++;
		pthread_cond_wait(&(rwstruct->writerP), &(rwstruct->read_write_lock));
	}

	rwstruct->pendW--;
	rwstruct->writer++;
	pthread_mutex_unlock(&(rwstruct->read_write_lock));
}

void mylib_rwlock_unlock(mylib_rwlock_t *rwstruct) {

	pthread_mutex_lock(&(rwstruct->read_write_lock));
	if (rwstruct->writer > 0) {
		rwstruct->writer = 0;
	}
	else if (rwstruct->reader > 0) {
		rwstruct->reader--;
	}
	pthread_mutex_unlock(&(rwstruct->read_write_lock));
	if ((rwstruct->reader == 0) && (rwstruct->pendW > 0)) {
		pthread_cond_signal(&(rwstruct->writerP));
	}
	else if (rwstruct->reader > 0) {
		pthread_cond_broadcast(&(rwstruct->readerP));
	}
}

double get_seconds() {
	struct timeval tvs;
	//      struct timezone tzs;

	gettimeofday(&tvs, NULL);

	return((double)tvs.tv_sec + (double)(tvs.tv_usec * 1.e-6));
}