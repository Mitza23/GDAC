#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "lib/mutex.h"
#include "lib/barrier.h"
#include "lib/ult.h"
#include "lib/util.h"

#define MAX_SLEEP 5

tid_t m1;
tid_t m2;
tid_t m3;
tid_t barrier;

void display_deadlocks() {
    bool deadlock_detected = false;

    // Check for deadlocks in mutex waiting cycles
    for (size_t i = 0; i < mutex_count; i++) {
        ult_mutex_t* m1 = &mutexes[i];
        if (-1 == m1->holder_id) {
            continue;
        }
        for (size_t j = i; j < mutex_count; j++) {
            ult_mutex_t* m2 = &mutexes[j];
            if (-1 == m2->holder_id || m1->holder_id == m2->holder_id) {
                continue;
            }

            if (m1->waiting_threads[m2->holder_id] &&
                m2->waiting_threads[m1->holder_id]) {
                // Two different threads waiting for the same lock
                printf(
                    "[Deadlock] Thread %d -> mutex %d held by thread %d and thread "
                    "%d -> mutex %d held by %d\n",
                    m2->holder_id, m1->id, m1->holder_id, m1->holder_id, m2->id,
                    m2->holder_id);
                if (!deadlock_detected) {
                    deadlock_detected = true;
                }
            }
        }
    }

    // Check for possibly eventual deadlocks in barrier waiting by mutex holders
    for (size_t i = 0; i < mutex_count; i++) {
        ult_mutex_t* mutex = &mutexes[i];
        if (mutex->holder_id == -1) {
            continue;  // Skip unlocked mutexes
        }

        tid_t holder = mutex->holder_id;

        // Check if any thread is waiting for this mutex
        for (size_t t = 0; t < MAX_THREADS_COUNT; t++) {
            if (mutex->waiting_threads[t]) {
                tid_t waiting_thread = t;

                // Check if the holder is waiting at a barrier
                for (size_t b = 0; b < current_barrier_id; b++) {
                    ult_barrier_t* barrier = &barriers[b];
                    if (barrier->waiting_threads[holder]) {
						// Mutex holder is waiting at a barrier and another thread is waiting for the mutex
                        printf("[Possibly Eventual Deadlock] Thread %lu is waiting for mutex %d held by thread %lu, which is waiting at barrier %d\n",
                            waiting_thread, mutex->id, holder, barrier->id);
                        deadlock_detected = true;
                    }
                }
            }
        }
    }

    if (!deadlock_detected) {
        printf("No deadlocks detected\n");
    }
}


void* f1(void* arg) {
  int random_time;

  while (1) {
    srand(time(NULL));
    random_time = rand() % MAX_SLEEP;
    printf("[%s]Sleeping for %d seconds\n", __FUNCTION__, random_time);
    ms_sleep(random_time * 1000);
    printf("[%s]Trying to acquire mutex1 (holding none)\n", __FUNCTION__);
    ult_mutex_lock(m1);
    printf("[%s]Acquired mutex1\n", __FUNCTION__);

    random_time = rand() % MAX_SLEEP;
    printf("[%s]Sleeping for %d seconds\n", __FUNCTION__, random_time);
    sleep(random_time);

    printf("[%s]Trying to acquire mutex2 (holding mutex1) \n", __FUNCTION__);
    ult_mutex_lock(m2);
    printf("[%s]Acquired mutex2\n\n", __FUNCTION__);

    ult_mutex_unlock(m2);
    ult_mutex_unlock(m1);
  }

  return NULL;
}

void* f2(void* arg) {
  int random_time;

  while (1) {
    srand(time(NULL));
    random_time = rand() % MAX_SLEEP;
    printf("[%s]Sleeping for %d seconds\n", __FUNCTION__, random_time);
    ms_sleep(random_time * 1000);
    printf("[%s]Trying to acquire mutex2 (holding none)\n", __FUNCTION__);
    ult_mutex_lock(m2);
    printf("[%s]Acquired mutex2\n", __FUNCTION__);

    random_time = rand() % MAX_SLEEP;
    printf("[%s]Sleeping for %d seconds\n", __FUNCTION__, random_time);
    sleep(random_time);

    printf("[%s]Trying to acquire mutex1 (holding mutex2) \n", __FUNCTION__);
    ult_mutex_lock(m1);
    printf("[%s]Acquired mutex1\n\n", __FUNCTION__);

    ult_mutex_unlock(m1);
    ult_mutex_unlock(m2);
  }
  return NULL;
}

void* f3(void* arg) {
    while (1) {
      
        printf("[%s]Trying to acquire mutex3 (holding none)\n", __FUNCTION__);
        ult_mutex_lock(m3);
        printf("[%s]Acquired mutex3\n", __FUNCTION__);

        printf("[%s]Wait at the barrier \n", __FUNCTION__);
        ult_barrier_wait(barrier);

    }
    return NULL;
}

void* f4(void* arg) {
    while (1) {
        ms_sleep(2000);
        printf("[%s]Trying to acquire mutex3 (holding none)\n", __FUNCTION__);
        ult_mutex_lock(m3);
       
        ult_mutex_unlock(m3);
    }
    return NULL;
}

void setup_barrier_test() {
    int status = ult_mutex_init(&m3);
    if (0 != status) {
        printf("Failed to initialize the ULT mutex: %s\n", strerror(errno));
        exit(status);
    }

    status = ult_barrier_init(&barrier, 3);
    if (0 != status) {
        printf("Failed to initialize the ULT barrier: %s\n", strerror(errno));
        exit(status);
    }

}

void setup_mutex_test() {
    int status = ult_mutex_init(&m1);
    if (0 != status) {
        printf("Failed to initialize the ULT mutex: %s\n", strerror(errno));
        exit(status);
    }

    status = ult_mutex_init(&m2);
    if (0 != status) {
        printf("Failed to initialize the ULT mutex: %s\n", strerror(errno));
        exit(status);
    }
}


void start_mutex_test(tid_t* t1, tid_t* t2) {
    int status = ult_create(t1, &f1, NULL);
    if (0 != status) {
        printf("Failed to create the ULT thread: %s\n", strerror(errno));
        exit(status);
    }

    status = ult_create(t2, &f2, NULL);
    if (0 != status) {
        printf("Failed to create the ULT thread: %s\n", strerror(errno));
        exit(status);
    }
}

void start_barrier_test(tid_t* t1, tid_t* t2) {
    int status = ult_create(t1, &f3, NULL);
    if (0 != status) {
        printf("Failed to create the ULT thread: %s\n", strerror(errno));
        exit(status);
    }

    status = ult_create(t2, &f4, NULL);
    if (0 != status) {
        printf("Failed to create the ULT thread: %s\n", strerror(errno));
        exit(status);
    }
}

int main() {
  tid_t t1;
  tid_t t2;

  int status = ult_init(100000);
  if (0 != status) {
    printf("Failed to initialize the ULT lib: %s\n", strerror(errno));
    exit(status);
  }

  /*setup_mutex_test();
  start_mutex_test(&t1, &t2);*/


  setup_barrier_test();
  start_barrier_test(&t1, &t2);

  

  while (1) {
	  display_deadlocks();
	  sleep(100);
  }

  status = ult_join(t1, NULL);
  if (0 != status) {
    printf("Failed to join the ULT thread: %s\n", strerror(errno));
    exit(status);
  }

  status = ult_join(t2, NULL);
  if (0 != status) {
    printf("Failed to join the ULT thread: %s\n", strerror(errno));
    exit(status);
  }

  return 0;
}