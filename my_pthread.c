#include "my_pthread.h"
#include <sys/time.h>
#include <signal.h>
#include <string.h>

#define STACK_SIZE 32768


// static variables
static my_pthread_t tid_counter = 0;
static my_pthread_tcb tcbs[500];
static int activated = -1;
static struct sigaction sa;
static struct itimerval timer;
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

sigset_t block_mask;
sigemptyset (&block_mask);
sigaddset (&block_mask, SIGPROF);
sigprocmask (SIG_BLOCK, &block_mask, NULL);

if (thread_queue->head->next != NULL) {

  int next = thread_queue->head->next->thread;
  int curr = thread_queue->head->thread;

  my_pthread_t temp = dequeue(thread_queue);
  if(tcbs[curr].status == RUNNABLE) {
    sigprocmask (SIG_UNBLOCK, &block_mask, NULL);
    enqueue(thread_queue, temp);
    swapcontext(&tcbs[curr].context, &tcbs[next].context);
  }
  else {
    sigprocmask (SIG_UNBLOCK, &block_mask, NULL);
    swapcontext(&tcbs[curr].context, &tcbs[next].context);
  }

}


}

void timer_init() {
  memset(&sa, 0, sizeof (sa));
  sa.sa_handler = &schedule;
  sigaction (SIGPROF, &sa, NULL);
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = TIME_QUANTUM_MS;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = TIME_QUANTUM_MS;
  setitimer (ITIMER_PROF, &timer, NULL);
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
  getcontext(&ucp);
  void *stack = malloc(STACK_SIZE);
  ucp.uc_link = NULL;
  ucp.uc_stack.ss_sp = stack;
  ucp.uc_stack.ss_size = STACK_SIZE;
  ucp.uc_stack.ss_flags = 0;
  makecontext(&ucp, (void*)(function), 0);

  my_pthread_tcb tcb = {tid_counter, RUNNABLE, ucp, NULL};
  tcbs[tid_counter] = tcb;
  enqueue(thread_queue, tid_counter);
  tid_counter++;

}

/* Give up the CPU and allow the next thread to run.
 */
void my_pthread_yield(){

  // Implement Here
  sigset_t block_mask;
  sigemptyset (&block_mask);
  sigaddset (&block_mask, SIGPROF);
  sigprocmask (SIG_BLOCK, &block_mask, NULL);

  schedule(1);

  sigprocmask (SIG_UNBLOCK, &block_mask, NULL);

}

/* The calling thread will not continue until the thread with tid thread
 * has finished executing.
 */
void my_pthread_join(my_pthread_t thread){

  // Implement Here //
  sigset_t block_mask;
  sigemptyset (&block_mask);
  sigaddset (&block_mask, SIGPROF);
  sigprocmask (SIG_BLOCK, &block_mask, NULL);

  if (tcbs[thread].status != FINISHED) {
    my_pthread_yield();
  }

  sigprocmask (SIG_UNBLOCK, &block_mask, NULL);

}


/* Returns the thread id of the currently running thread
 */
my_pthread_t my_pthread_self(){

  // Implement Here //
  return thread_queue->head->thread; // temporary return, replace this

}

/* Thread exits, setting the state to finished and allowing the next thread
 * to run.
 */
void my_pthread_exit(){

  // Implement Here //

  sigset_t block_mask;
  sigemptyset (&block_mask);
  sigaddset (&block_mask, SIGPROF);
  sigprocmask (SIG_BLOCK, &block_mask, NULL);

  tcbs[my_pthread_self()].status = FINISHED;
  schedule(0);

  sigprocmask (SIG_UNBLOCK, &block_mask, NULL);

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
