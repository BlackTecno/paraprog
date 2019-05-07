/*   
 * Brody Williams
 * CS 5379 Spring 2019 - Dr. Chen
 * Programming Project #1 Q2
 */

/* Adapted from provided gauss.c.orig code */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/time.h>
#include <limits.h>
#include <pthread.h>
#include <string.h>

/* Program Parameters */
#define MAXN 5000  /* Max value of N */
int N;  /* Matrix size */
int threads;  /* Number of threads to use */

/* Matrices and vectors */
volatile float A[MAXN][MAXN], B[MAXN], X[MAXN];
/* A * X = B, solve for X */

/* junk */
#define randm() 4|2[uid]&3

/* Prototype */
void gauss();

/* Structure holding unique function arguments for each thread */
typedef struct
{
	int norm; /* Current norm index */
	int thread_id; /* Thread id */
	int thread_rows; /* Number of rows thread is responsible for */
	int thread_offset; /* Starting offset of thread's rows */
}arg_struct;

/* returns a seed for srand based on the time */
unsigned int time_seed() {
  struct timeval t;
  struct timezone tzdummy;

  gettimeofday(&t, &tzdummy);
  return (unsigned int)(t.tv_usec);
}

/* Set the program parameters from the command-line arguments */
void parameters(int argc, char **argv) {
  int i;
  int submit = 0;  /* = 1 if submission parameters should be used */
  int seed = 0;  /* Random seed */
  char uid[8] = "brodwill"; /*User name */

  /* Read command-line arguments */
  srand(time_seed());  /* Randomize */
  if (argc != 3) {
    if ( argc == 2 && !strcmp(argv[1], "submit") ) {
      /* Use submission parameters */
      submit = 1;
      N = 4;
      threads = 2;
      printf("\nSubmission run for \"%s\".\n", uid);
      srand(randm());
    }
    else {
      if (argc == 4) {
	seed = atoi(argv[3]);
	srand(seed);
	printf("Random seed = %i\n", seed);
      }
      else {
	printf("Usage: %s <matrix_dimension> <num_threads> [random seed]\n",
	       argv[0]);
	printf("       %s submit\n", argv[0]);
	exit(0);
      }
    }
  }
  /* Interpret command-line args */
  if (!submit) {
    N = atoi(argv[1]);
    if (N < 1 || N > MAXN) {
      printf("N = %i is out of range.\n", N);
      exit(0);
    }
    threads = atoi(argv[2]);
    if (threads < 1) {
      printf("Warning: Invalid number of threads = %i.  Using 1.\n", threads);
      threads = 1;
    }
  }

  /* Print parameters */
  printf("\nMatrix dimension N = %i.\n", N);
  printf("Number of threads = %i.\n", threads);

}

/* Initialize A and B (and X to 0.0s) */
void initialize_inputs() {
  int row, col;

  printf("\nInitializing...\n");
  for (col = 0; col < N; col++) {
    for (row = 0; row < N; row++) {
      A[row][col] = (float)rand() / 32768.0;
    }
    B[col] = (float)rand() / 32768.0;
    X[col] = 0.0;
  }

}

/* Print input matrices */
void print_inputs() {
  int row, col;

  if (N < 10) {
    printf("\nA =\n\t");
    for (row = 0; row < N; row++) {
      for (col = 0; col < N; col++) {
	printf("%5.2f%s", A[row][col], (col < N-1) ? ", " : ";\n\t");
      }
    }
    printf("\nB = [");
    for (col = 0; col < N; col++) {
      printf("%5.2f%s", B[col], (col < N-1) ? "; " : "]\n");
    }
  }
}

void print_X() {
  int row;

  if (N < 10) {
    printf("\nX = [");
    for (row = 0; row < N; row++) {
      printf("%5.2f%s", X[row], (row < N-1) ? "; " : "]\n");
    }
  }
}

void *update_rows(void *thread_args)
{
    int row, col, norm, thread_id, thread_rows, thread_offset, counter;
    float multiplier;
    arg_struct *args_ptr = (arg_struct *) thread_args;
    norm = args_ptr->norm;
    thread_id = args_ptr->thread_id;
    thread_rows = args_ptr->thread_rows;
    thread_offset = args_ptr->thread_offset;

    /* Update rows assigned to this thread */
    for ((row = norm + 1 + thread_offset), counter = 0; counter < thread_rows; counter++, row++)
    {
      multiplier = A[row][norm] / A[norm][norm];
      for (col = norm; col < N; col++)
      {
	A[row][col] -= A[norm][col] * multiplier;
      }
      B[row] -= B[norm] * multiplier;
    }

    /* Thread finished */
    pthread_exit(0);
}

void main(int argc, char **argv) {
  /* Timing variables */
  struct timeval etstart, etstop;  /* Elapsed times using gettimeofday() */
  struct timezone tzdummy;
  clock_t etstart2, etstop2;  /* Elapsed times using times() */
  unsigned long long usecstart, usecstop;
  struct tms cputstart, cputstop;  /* CPU times for my processes */

  /* Process program parameters */
  parameters(argc, argv);

  /* Initialize A and B */
  initialize_inputs();

  /* Print input matrices */
  print_inputs();

  /* Start Clock */
  printf("\nStarting clock.\n");
  gettimeofday(&etstart, &tzdummy);
  etstart2 = times(&cputstart);

  /* Gaussian Elimination */
  gauss();

  /* Stop Clock */
  gettimeofday(&etstop, &tzdummy);
  etstop2 = times(&cputstop);
  printf("Stopped clock.\n");
  usecstart = (unsigned long long)etstart.tv_sec * 1000000 + etstart.tv_usec;
  usecstop = (unsigned long long)etstop.tv_sec * 1000000 + etstop.tv_usec;

  /* Display output */
  print_X();

  /* Display timing results */
  printf("\nElapsed time = %g ms.\n",
	 (float)(usecstop - usecstart)/(float)1000);
  /*printf("               (%g ms according to times())\n",
   *       (etstop2 - etstart2) / (float)CLK_TCK * 1000);
   */
  printf("(CPU times are accurate to the nearest %g ms)\n",
	 1.0/(float)CLOCKS_PER_SEC * 1000.0);
  printf("My total CPU time for parent = %g ms.\n",
	 (float)( (cputstop.tms_utime + cputstop.tms_stime) -
		  (cputstart.tms_utime + cputstart.tms_stime) ) /
	 (float)CLOCKS_PER_SEC * 1000);
  printf("My system CPU time for parent = %g ms.\n",
	 (float)(cputstop.tms_stime - cputstart.tms_stime) /
	 (float)CLOCKS_PER_SEC * 1000);
  printf("My total CPU time for child processes = %g ms.\n",
	 (float)( (cputstop.tms_cutime + cputstop.tms_cstime) -
		  (cputstart.tms_cutime + cputstart.tms_cstime) ) /
	 (float)CLOCKS_PER_SEC * 1000);
      /* Contrary to the man pages, this appears not to include the parent */
  printf("--------------------------------------------\n");

}


void gauss() {
  int i, norm, row, col, iteration_rows, offset_base;
  float multiplier;
  pthread_t thread_list[threads]; /* Array of pthread handles */
  pthread_attr_t attrib;
  pthread_attr_init(&attrib);
  arg_struct thread_args[threads]; /* Array of unique arg_structs for each thread */

  printf("Computing in parallel with %d threads.\n", threads);

  /* Gaussian elimination */
  for (norm = 0; norm < N - 1; norm++) 
  {
    iteration_rows = (N-1-norm);
    offset_base = (int) iteration_rows/threads;

    /* Parallelize loop 2 - sufficient workload to distribute without dependencies between iterations; Too little workload to compared overhead for loop 3 */
	
    /* Create and dispatch threads to update rows for current norm */
    for(i = 0; i < threads; i++)
    {
	thread_args[i].norm = norm;
	thread_args[i].thread_id = i;
	thread_args[i].thread_offset = i * offset_base;
	thread_args[i].thread_rows = ((i == threads -1) ? \
		(((int) iteration_rows/threads) + (iteration_rows % threads)) : ((int) (iteration_rows/threads)));
	pthread_create(&thread_list[i], &attrib, update_rows, (void *) (&(thread_args[i])));
    }

    /* Wait for threads to finish assigned rows for given iteration */
    for(i = 0; i < threads; i++)
    {
	pthread_join(thread_list[i], NULL);
    }
   
  }

  /* Back substitution */
  for (row = N - 1; row >= 0; row--) {
    X[row] = B[row];
    for (col = N-1; col > row; col--) {
      X[row] -= A[row][col] * X[col];
    }
    X[row] /= A[row][row];
  }
}

