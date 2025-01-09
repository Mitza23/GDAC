#include "mutex.h"

#include <errno.h>
#include <stdio.h>

#include "util.h"


ult_mutex_t mutexes[MAX_THREADS_COUNT];
size_t mutex_count = 0;

int ult_mutex_init(tid_t* mutex_id) {
  if (MAX_THREADS_COUNT - 1 == mutex_count) {
    return EXIT_FAILURE;
  }

  ult_mutex_t* m = &mutexes[mutex_count];
  m->id = mutex_count;
  *mutex_id = mutex_count;
  m->holder_id = -1;
  m->waiting_threads_count = 0;

  mutex_count++;

  return EXIT_SUCCESS;
}

int ult_mutex_lock(tid_t mutex_id) {
  block_signals();
  tid_t new_holder = ult_self();
  ult_mutex_t* m = &mutexes[mutex_id];

  if (m->holder_id == new_holder) {
    unblock_signals();
    return EXIT_SUCCESS;
  }

  // Mutex is already locked by another thread
  if (-1 != m->holder_id) {
    m->waiting_threads_count++;
    m->waiting_threads[new_holder] = true;

    do {
      unblock_signals();
      ult_yield();
	  block_signals();
    } while (-1 != m->holder_id);
  }

  m->holder_id = new_holder;
  unblock_signals();

  return EXIT_SUCCESS;
}

int ult_mutex_unlock(tid_t mutex_id) {
  block_signals();
  tid_t self = ult_self();
  ult_mutex_t* m = &mutexes[mutex_id];

  if (m->holder_id != self) {
    // The current thread does not own the mutex.
    unblock_signals();
    errno = EPERM;
    return EXIT_FAILURE;
  }

  m->holder_id = -1;
  for (size_t i = 0; i < MAX_THREADS_COUNT; i++) {
    m->waiting_threads[i] = false;
  }
  m->waiting_threads_count = 0;

  unblock_signals();

  return EXIT_SUCCESS;
}

int ult_mutex_destroy(tid_t mutex_id) {
  ult_mutex_t* m = &mutexes[mutex_id];

  if (-1 != m->holder_id) {
    errno = EBUSY;
    return EXIT_FAILURE;
  }

  m->id = -1;

  return EXIT_SUCCESS;
}
