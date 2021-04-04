/*
 *	File	: pc.c
 *
 *	Title	: Demo Producer/Consumer.
 *
 *	Short	: A solution to the producer consumer problem using
 *		pthreads.	
 *
 *	Long 	:
 *
 *	Author	: Andrae Muys
 *
 *	Date	: 18 September 1997
 *
 *	Revised	:
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>

#define QUEUESIZE 20
#define LOOP 1000000 //number of repeats
#define num_threads 2048 //number of threads 
//#define title "times_6_2048.txt"

int num_producers =0, num_consumers = 0;  //number of producer and consumer functions created by all threads
struct timeval t1[LOOP], t2[LOOP];
float t_elapsed[LOOP];
//double time_for_txt[LOOP];


void *producer (void *args);
void *consumer (void *args);
void *findSin(int arg);
void *message(int arg );
void *sq_root(int arg);
void *intToHex(int arg);
void *findLog(int arg);
void *random_function(int i);

typedef struct {
  void * (*work)(void *);
  void * arg;
} workFunction;

typedef struct {
  int buf[QUEUESIZE];
  long head, tail;
  int full, empty;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
  workFunction work[QUEUESIZE];  //
} queue;

queue *queueInit (void);
void queueDelete (queue *q);
void queueAdd (queue *q, int in);
void queueDel (queue *q, int *out);

int main ()
{
  queue *fifo;  
  pthread_t tidpro[num_threads], tidcon[num_threads]; //intialize threads
  fifo = queueInit ();   // initialize queue
  if (fifo ==  NULL) {
    fprintf (stderr, "main: Queue Init failed.\n");
    exit (1);
  }

  //Creation of #num_threads new threads to execute producer and consumer functions
  for (int i=0; i<num_threads; i++) {
    pthread_create(&tidpro[i], NULL, producer, fifo);
    printf("Creating producer #%d\n", i);
    pthread_create(&tidcon[i], NULL, consumer, fifo);
    printf("Creating consumer #%d\n", i);
  }

  //terminate the threads
  for(int i=0;i<num_threads;i++){
    pthread_join(tidpro[i],NULL);
    pthread_join(tidcon[i],NULL);
  }
  /*
    FILE *f = fopen(title, "w");
    if (f == NULL) {
      printf("Error opening file!\n");
      exit(1);
    }
    for (int i=0; i<num_consumers;i++){
      fprintf(f,"%f\n", time_for_txt[i]);

    }
  */

  queueDelete (fifo); // delete queue

  return 0;
}

/* function producer
    Generates data and writes it to the queue 
*/
void *producer (void *q)
{
  queue *fifo;
  int i;

  fifo = (queue *)q;

  //number of functions is divided in equal number of executions for each thread
  for (i = 0; i < LOOP/num_threads; i++) {
    
    pthread_mutex_lock (fifo->mut);
    
    while (fifo->full) {
      //printf ("producer: queue FULL.\n");
      pthread_cond_wait (fifo->notFull, fifo->mut);
    }

    num_producers++;

    queueAdd (fifo, i);
    pthread_mutex_unlock (fifo->mut);
    pthread_cond_signal (fifo->notEmpty);
  }
  /*for (i = 0; i < LOOP; i++) {
    pthread_mutex_lock (fifo->mut);
    while (fifo->full) {
      printf ("producer: queue FULL.\n");
      pthread_cond_wait (fifo->notFull, fifo->mut);
    }
    queueAdd (fifo, i);
    pthread_mutex_unlock (fifo->mut);
    pthread_cond_signal (fifo->notEmpty);
    usleep (200000);
  }*/
  
  return (NULL);
}

/* function producer
    Reads the data from the queue, times the communication, executes the function 
    that has been stored in the queue and then removes them. 
*/
void *consumer (void *q)
{
  queue *fifo;
  int i, d;
  // float t_elapsed[LOOP];

  fifo = (queue *)q;
  
  //for (i = 0; i < LOOP/num_threads; i++) {

  while (1) {  
    //srand(time(NULL));
    pthread_mutex_lock (fifo->mut);

    while (fifo->empty) {
      //printf ("consumer: queue EMPTY.\n");
      pthread_cond_wait (fifo->notEmpty, fifo->mut);
    }

    queueDel (fifo, &d);
    
    num_consumers++;
    
    pthread_mutex_unlock (fifo->mut);
    pthread_cond_signal (fifo->notFull);
    
  }
  /*  for (i = 0; i < LOOP; i++) {
      pthread_mutex_lock (fifo->mut);
      while (fifo->empty) {
        printf ("consumer: queue EMPTY.\n");
        pthread_cond_wait (fifo->notEmpty, fifo->mut);
      }
      queueDel (fifo, &d);
      pthread_mutex_unlock (fifo->mut);
      pthread_cond_signal (fifo->notFull);
      printf ("consumer: recieved %d.\n", d);
      usleep (50000);
  }
  */
  
  return (NULL);
}


queue *queueInit (void)
{
  queue *q;

  q = (queue *)malloc (sizeof (queue));
  if (q == NULL) return (NULL);

  q->empty = 1;
  q->full = 0;
  q->head = 0;
  q->tail = 0;
  q->mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
  pthread_mutex_init (q->mut, NULL);
  q->notFull = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notFull, NULL);
  q->notEmpty = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notEmpty, NULL);
	
  return (q);
}

void queueDelete (queue *q)
{
  pthread_mutex_destroy (q->mut);
  free (q->mut);	
  pthread_cond_destroy (q->notFull);
  free (q->notFull);
  pthread_cond_destroy (q->notEmpty);
  free (q->notEmpty);
  free (q);
}

void queueAdd (queue *q, int in)
{
  //starting timer
  gettimeofday(&t1[num_producers-1], NULL);

  q->buf[q->tail] = in;

   //pick a number based on the current addition to the queue
  q->work[q->tail].arg = (void*)q->tail+in;
    //pick a random function 
  q->work[q->tail].work = random_function(q->tail);

  q->tail++;
  if (q->tail == QUEUESIZE)
    q->tail = 0;
  if (q->tail == q->head)
    q->full = 1;
  q->empty = 0;

  return;
}

void queueDel (queue *q, int *out)
{
  
  *out = q->buf[q->head];

  //gets the time between the adding of the data and reading them from the queue
  gettimeofday(&t2[num_consumers], NULL);
  t_elapsed[num_consumers] = (double) ((t2[num_consumers].tv_usec - t1[num_consumers].tv_usec)/1.0e6 + t2[num_consumers].tv_sec - t1[num_consumers].tv_sec);
  printf("Time for the communication #%i is %f s\n",num_consumers,  t_elapsed[num_consumers]);
  //time_for_txt[num_consumers] = t_elapsed[num_consumers];
  
  //execution of the selected function
  (q->work[q->head].work)(q->work[q->head].arg);
  
  q->head++;
  if (q->head == QUEUESIZE)
    q->head = 0;
  if (q->head == q->tail)
    q->empty = 1;
  q->full = 0;
  return;
}


void * findSin(int arg) {   
  double g = (double)arg/rand();
  //printf("\nThe Sine is %f\n",g);
}
void * message(int arg ) {
   //printf("\nHello from function 'message'  \n");
}
void * sq_root(int arg) {
  //printf("the square root of %d is %f",arg,sqrt(arg));
  // /printf("\n");
}
void * intToHex(int arg) {
  //printf("\nThe number %d, turned to hexadecimal is: %x\n", arg,arg);
}
void * findLog(int arg) {
  //printf("\nlog2 of %d is %f\n",arg, (double)log2(arg));  
}

void *random_function(int i){
  
  if (i%5==0){
    if(i%2 == 0) {
      return &findSin;
    } else {
      return &message;
    }
 } else {
    if(i%2 ==0){
      if(i%3 ==0 ){
        return &sq_root;
      } else {
        return &intToHex;
      }
    } else {
      return &findLog;
    }
  }
} 
