#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "lib/ult.h"
#include "lib/util.h"

#define THREAD_COUNT 10

void* f(void* arg) {
  tid_t self = ult_self();
  for (size_t i = 0; i < 100; i++) {
    ms_sleep(100);
    printf("Hello from thread %lu (%lu)\n", self, i);
  }

  return (void*)(self * 11);
}

int main() {
  tid_t threads[THREAD_COUNT];
  tid_t retvals[THREAD_COUNT];

  int status = ult_init(100000);
  if (0 != status) {
    printf("Failed to initialize the ULT lib: %s\n", strerror(errno));
    exit(status);
  }

  for (size_t i = 0; i < THREAD_COUNT; i++) {
    status = ult_create(&threads[i], &f, NULL);
    if (0 != status) {
      printf("Failed to create the ULT thread: %s\n", strerror(errno));
      exit(status);
    }
  }

  for (size_t i = 0; i < THREAD_COUNT; i++) {
    status = ult_join(threads[i], (void**)&retvals[i]);
    if (0 != status) {
      printf("Failed to join the ULT thread: %s\n", strerror(errno));
      exit(status);
    }
    printf("Joined thread %lu with return value %lu\n", threads[i], retvals[i]);
  }

  return 0;
}