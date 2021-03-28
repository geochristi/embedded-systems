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

#define QUEUESIZE 20
#define LOOP 1000000
#define num_threads 1024

int producerthreads =0, consumerthreads = 0;

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
  workFunction work[LOOP];
} queue;

queue *queueInit (void);
void queueDelete (queue *q);
void queueAdd (queue *q, int in);
void queueDel (queue *q, int *out);

int main ()
{
  queue *fifo;  
  //pthread_t pro, con;  //intialize threads
  pthread_t tidpro[num_threads], tidcon[num_threads]; 

  fifo = queueInit ();   // initialize queue
  if (fifo ==  NULL) {
    fprintf (stderr, "main: Queue Init failed.\n");
    exit (1);
  }

  for (int i=0; i<num_threads; i++) {
    pthread_create(&tidpro[i], NULL, producer, fifo);
    printf("Creating producer #%d\n", i);
    pthread_create(&tidcon[i], NULL, consumer, fifo);
    printf("Creating consumer #%d\n", i);
  }


  for(int i=0;i<num_threads;i++){
    pthread_join(tidpro[i],NULL);

    pthread_join(tidcon[i],NULL);
  }
  // pthread_create (&pro, NULL, producer, fifo);
  // pthread_create (&con, NULL, consumer, fifo);
  // pthread_join (pro, NULL);
  // pthread_join (con, NULL);
  queueDelete (fifo);

  return 0;
}

void *producer (void *q)
{
  queue *fifo;
  int i;

  fifo = (queue *)q;


  //number of functions is divided in equal number of functions for each thread
  for (i = 0; i < LOOP/num_threads; i++) {
    pthread_mutex_lock (fifo->mut);
    while (fifo->full) {
     // printf ("producer: queue FULL.\n");
      pthread_cond_wait (fifo->notFull, fifo->mut);
    }
    producerthreads++;

    //printf("producerthreads %d\n",producerthreads);
    //printf("producerthreads/loop %d\n", producerthreads%LOOP);
    queueAdd (fifo, i);
    pthread_mutex_unlock (fifo->mut);
    pthread_cond_signal (fifo->notEmpty);
    // producerthreads++;
    //usleep (100000);
  }
  // for (i = 0; i < LOOP; i++) {
  //   pthread_mutex_lock (fifo->mut);
  //   while (fifo->full) {
  //     printf ("producer: queue FULL.\n");
  //     pthread_cond_wait (fifo->notFull, fifo->mut);
  //   }
  //   queueAdd (fifo, i);
  //   pthread_mutex_unlock (fifo->mut);
  //   pthread_cond_signal (fifo->notEmpty);
  //   usleep (200000);
  // }
  return (NULL);
}

void *consumer (void *q)
{
  queue *fifo;
  int i, d;
  //pid_t tid = gettid();
 // printf("my id is %d\n", tid);
  fifo = (queue *)q;
  //while (1) {  //it never ends
  //for (i = 0; i < LOOP; i++) {
  for (i =0; i< LOOP/num_threads; i++) {
    pthread_mutex_lock (fifo->mut);
    while (fifo->empty) {
      printf ("consumer: queue EMPTY.\n");
      pthread_cond_wait (fifo->notEmpty, fifo->mut);
    }


    // printf("consumerthreads %d\n", consumerthreads);
    // (*fifo->work[consumerthreads].work)(fifo->work[consumerthreads].arg);

    // consumerthreads++;
    queueDel (fifo, &d);
    consumerthreads++;

    
    pthread_mutex_unlock (fifo->mut);
    pthread_cond_signal (fifo->notFull);
    
    // printf("consumerthreads %d\n", consumerthreads);
    (*fifo->work[consumerthreads-1].work)(fifo->work[consumerthreads-1].arg);


    //printf ("consumer: recieved %d.\n", d);
    //usleep(200000);
  }
  // for (i = 0; i < LOOP; i++) {
  //   pthread_mutex_lock (fifo->mut);
  //   while (fifo->empty) {
  //     printf ("consumer: queue EMPTY.\n");
  //     pthread_cond_wait (fifo->notEmpty, fifo->mut);
  //   }
  //   queueDel (fifo, &d);
  //   pthread_mutex_unlock (fifo->mut);
  //   pthread_cond_signal (fifo->notFull);
  //   printf ("consumer: recieved %d.\n", d);
  //   usleep (50000);
  // }
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
  q->buf[q->tail] = in;
  q->tail++;
  if (q->tail == QUEUESIZE)
    q->tail = 0;
  if (q->tail == q->head)
    q->full = 1;
  q->empty = 0;
   printf("producer %d\n", producerthreads);
  // printf("producer/loop %d\n", producerthreads%LOOP);

  // printf("tail+in %d\n", q->tail+in);
  
  //pick a number based on the current addition to the queue
  q->work[producerthreads-1].arg = (void*)q->tail+in;

  //pick a random function 
  q->work[producerthreads-1].work = random_function(q->tail);
  return;
}

void queueDel (queue *q, int *out)
{
  *out = q->buf[q->head];

  q->head++;
  if (q->head == QUEUESIZE)
    q->head = 0;
  if (q->head == q->tail)
    q->empty = 1;
  q->full = 0;

  return;
}


void * findSin(int arg) {   
  double g;
  double k = (double)arg/(LOOP*QUEUESIZE);
  for(int i=0;i<arg;i++){
    g = sin(k) + g;
  }
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
