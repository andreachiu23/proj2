#include "my_pthread.h"
#include <sys/time.h>
#include <signal.h>
#include <string.h>

#define STACK_SIZE 32768



// static variables
static my_pthread_t tid_counter = 0;
static my_pthread_tcb tcbs[500];
int activated = -1;
struct sigaction sa;
struct itimerval timer;
static queue *thread_queue;


void queue_init(queue *q);
void enqueue(queue *q, my_pthread_t thread);
my_pthread_t dequeue(queue *q);


/* Scheduler State */
 // Fill in Here //


/* Scheduler Function
 * Pick the next runnable thread and swap contexts to start executing
 */
void schedule(int signum){

  // Implement Here

  /*
  static int count = 0;
  printf("timer expired %d times\n", ++count);
  */

/*
  printf("%d ", thread_queue->head->thread);
  ucontext_t oucp = tcbs[thread_queue->head->thread].context;
  printf("%d\n", oucp);

  printf("%d ", thread_queue->head->next->thread);
  ucontext_t ucp = tcbs[thread_queue->head->next->thread].context;
  printf("%d\n", ucp);
  */




  /*swapcontext(ucontext_t *oucp, const ucontext_t *ucp)
  This function saves the current execution context into the context pointed to by oucp and starts
  resumes the execution context of the ucontext pointed to by ucp*/

  ucontext_t oucp = tcbs[thread_queue->head->thread].context;
  ucontext_t ucp = tcbs[thread_queue->head->next->thread].context;
  swapcontext(&oucp, &ucp);
  my_pthread_t temp = dequeue(thread_queue);
  enqueue(thread_queue, temp);

}

void timer_init() {
  memset(&sa, 0, sizeof (sa));
  sa.sa_handler = &schedule;
  sigaction (SIGVTALRM, &sa, NULL);
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = TIME_QUANTUM_MS;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = TIME_QUANTUM_MS;
  setitimer (ITIMER_VIRTUAL, &timer, NULL);
}


/* Create a new TCB for a new thread execution context and add it to the queue
 * of runnable threads. If this is the first time a thread is created, also
 * create a TCB for the main thread as well as initalize any scheduler state.
 */
void my_pthread_create(my_pthread_t *thread, void*(*function)(void*), void *arg){

  // Implement Here
  if (activated == -1) {
    ucontext_t main;
    getcontext(&main);

    thread_queue = (queue*)malloc(sizeof(queue));
    queue_init(thread_queue);
    enqueue(thread_queue, tid_counter);

    my_pthread_tcb tcb = {tid_counter, RUNNABLE, main, NULL};
    tcbs[tid_counter] = tcb;
    tid_counter++;
    activated = 1;
    timer_init();
  }

  ucontext_t ucp;
  void *stack = malloc(STACK_SIZE);
  ucp.uc_link = NULL;
  ucp.uc_stack.ss_sp = stack;
  ucp.uc_stack.ss_size = STACK_SIZE;
  ucp.uc_stack.ss_flags = 0;
  makecontext(&ucp, (void*)(&function), 0);

  my_pthread_tcb tcb = {tid_counter, RUNNABLE, ucp, NULL};
  tcbs[tid_counter] = tcb;
  enqueue(thread_queue, tid_counter);
  tid_counter++;

}

/* Give up the CPU and allow the next thread to run.
 */
void my_pthread_yield(){

  // Implement Here

}

/* The calling thread will not continue until the thread with tid thread
 * has finished executing.
 */
void my_pthread_join(my_pthread_t thread){

  // Implement Here //

}


/* Returns the thread id of the currently running thread
 */
my_pthread_t my_pthread_self(){

  // Implement Here //

  return 0; // temporary return, replace this

}

/* Thread exits, setting the state to finished and allowing the next thread
 * to run.
 */
void my_pthread_exit(){

  // Implement Here //

}



void queue_init(queue *q) {
	q -> head = NULL;
	q -> tail = NULL;
	q -> size = 0;
}

void enqueue(queue *q, my_pthread_t thread) {
  queue_node *new_node = (queue_node *)malloc(sizeof(queue_node));
  new_node->thread = thread;
  new_node->next = NULL;

  if (q->size == 0) {
    q->head = new_node;
    q->tail = new_node;
    (q->size)++;
	}
	else {
		queue_node *old_tail = q->tail;
		old_tail->next = new_node;
		q->tail = new_node;
		(q-> size)++;
	}

}

my_pthread_t dequeue(queue *q) {
  if (q->size == 0) {
    return;
  }
  queue_node *new_front = q->head->next;
  my_pthread_t removed = q->head->thread;
  free(q->head);
  q->head = new_front;
  (q->size)--;
  return removed;

}
