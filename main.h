#ifndef MAIN_H
#define MAIN_H

#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define NUM_THREADS 10
#define N 5 // Number of parking spaces

extern pthread_t threads_ids[NUM_THREADS];
extern pthread_mutex_t mutex;
extern sem_t parking_semaphore;
extern int total_cars;
extern double total_wait_time;

void *access_parking_lot(void *arg);
void log_event(const char *event, int thread_id, time_t event_time);

#endif
