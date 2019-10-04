#include <stdio.h>
#include <stdlib.h>
#include "my_pthread.h"

void thread_run(){
  while(1){
    printf("Thread Running %d\n", my_pthread_self());
    // printf("Thread running\n");
  }
}

int main(){

  my_pthread_t thread;

  my_pthread_create(&thread, (void*) thread_run, (void*) NULL);


  while(1) {

    printf("Main Thread Running\n");

  }



}
