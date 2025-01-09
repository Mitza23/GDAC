#ifndef ULT_H
#define ULT_H


#include <ucontext.h>

#include <stdbool.h>
#include <stdlib.h>

#define MAX_THREADS_COUNT 1000

typedef enum state_t { ULT_BLOCKED, ULT_READY, ULT_TERMINATED } state_t;

typedef unsigned long int tid_t;

typedef struct ult_t {
  tid_t tid;

  ucontext_t context;

  state_t state;
  void *(*start_routine)(void *);
  void *arg;
  void *retval;
} ult_t;


int ult_init(long quantum);


int ult_create(tid_t *thread_id, void *(*start_routine)(void *), void *arg);


int ult_join(tid_t thread_id, void **retval);


tid_t ult_self();

void ult_exit(void *retval);

void ult_yield(void);

#endif