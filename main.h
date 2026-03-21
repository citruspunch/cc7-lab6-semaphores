#ifndef MAIN_H
#define MAIN_H

#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#ifndef NUM_THREADS
#define NUM_THREADS 10
#endif

#ifndef N
#define N 5 // Number of parking spaces
#endif

extern pthread_t threads_ids[NUM_THREADS];
extern pthread_mutex_t mutex;
extern sem_t parking_semaphore;
extern int total_cars;
extern double total_wait_time;

// Dashboard monitoring structures
typedef struct {
  int occupied;          // 0 = free, 1 = occupied
  int car_id;            // ID of car occupying this space, -1 if free
  time_t occupied_since; // When this space was occupied
} ParkingSpace;

typedef struct {
  int car_id;
  time_t arrival_time;
  double wait_time;
  int status; // 0: waiting, 1: parked, 2: departed
} CarStatus;

extern ParkingSpace parking_spaces[N];
extern CarStatus car_statuses[NUM_THREADS];
extern int current_parked_count;
extern int current_waiting_count;
extern pthread_mutex_t dashboard_mutex;

void *access_parking_lot(void *arg);
void log_event(const char *event, int thread_id, time_t event_time);

#endif
