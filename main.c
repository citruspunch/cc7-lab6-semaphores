
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

pthread_t threads_ids[NUM_THREADS];
pthread_mutex_t mutex;

sem_t parking_semaphore; // Initialize the semaphore. Will be set to N in main

int total_cars = 0;           // Total number of cars that have parked.
double total_wait_time = 0.0; // Total wait time for all cars.

// Dashboard monitoring variables
ParkingSpace parking_spaces[N];
CarStatus car_statuses[NUM_THREADS];
int current_parked_count = 0;
int current_waiting_count = 0;
pthread_mutex_t dashboard_mutex;

static double elapsed_seconds(const struct timespec *start,
                              const struct timespec *end) {
  return (double)(end->tv_sec - start->tv_sec) +
         (double)(end->tv_nsec - start->tv_nsec) / 1000000000.0;
}

// Function to display the real-time dashboard
void display_dashboard() {
  // Copy all needed data while holding the mutex (minimize lock time)
  ParkingSpace spaces_copy[N];
  CarStatus statuses_copy[NUM_THREADS];
  int parked_copy, waiting_copy, total_cars_copy;
  double total_wait_time_copy;

  pthread_mutex_lock(&dashboard_mutex);
  memcpy(spaces_copy, parking_spaces, sizeof(parking_spaces));
  memcpy(statuses_copy, car_statuses, sizeof(car_statuses));
  parked_copy = current_parked_count;
  waiting_copy = current_waiting_count;
  total_cars_copy = total_cars;
  total_wait_time_copy = total_wait_time;
  pthread_mutex_unlock(&dashboard_mutex);

  printf("=== PARKING LOT REAL-TIME DASHBOARD ===\n\n");

  // Display parking spaces with consistent padding
  printf("PARKING SPACES (%d/%d occupied):\n", parked_copy, N);
  printf("+");
  for (int i = 0; i < N; i++) {
    printf("-------+");
  }
  printf("\n");
  for (int i = 0; i < N; i++) {
    if (spaces_copy[i].occupied) {
      printf("|  %-3d ", spaces_copy[i].car_id);
    } else {
      printf("|       ");
    }
  }
  printf("|\n");
  printf("+");
  for (int i = 0; i < N; i++) {
    printf("-------+");
  }
  printf("\n");
  for (int i = 0; i < N; i++) {
    printf("   [%d]  ", i);
  }
  printf("\n\n");

  // Display car statuses with consistent column widths
  printf("CAR STATUS:\n");
  printf("+-------+-----------+-------------+---------------------+\n");
  printf("| Car   | Status    | Wait Time   | Arrival Time        |\n");
  printf("+-------+-----------+-------------+---------------------+\n");

  int actual_waiting = 0;
  for (int i = 0; i < NUM_THREADS; i++) {
    char arrival_str[20];
    if (statuses_copy[i].arrival_time > 0) {
      strncpy(arrival_str, ctime(&statuses_copy[i].arrival_time), 19);
      arrival_str[19] = '\0';
    } else {
      strcpy(arrival_str, "--");
    }

    if (statuses_copy[i].status == 0) {
      actual_waiting++;
      printf("| %-5d | %-9s | %-11s | %-16s |\n", i, "WAITING", "--",
             arrival_str);
    } else if (statuses_copy[i].status == 1) {
      char wait_str[20];
      snprintf(wait_str, sizeof(wait_str), "%.2fs", statuses_copy[i].wait_time);
      printf("| %-5d | %-9s | %-11s | %-16s |\n", i, "PARKED", wait_str,
             arrival_str);
    } else if (statuses_copy[i].status == 2) {
      char wait_str[20];
      snprintf(wait_str, sizeof(wait_str), "%.2fs", statuses_copy[i].wait_time);
      printf("| %-5d | %-9s | %-11s | %-16s |\n", i, "DEPARTED", wait_str,
             arrival_str);
    } else {
      printf("| %-5d | %-9s | %-11s | %-16s |\n", i, "PENDING", "--", "--");
    }
  }
  printf("+-------+-----------+-------------+---------------------+\n\n");

  // Display statistics
  printf("STATISTICS:\n");
  printf("  Total cars processed: %d\n", total_cars_copy);
  if (total_cars_copy > 0) {
    printf("  Average wait time: %.3f seconds\n",
           total_wait_time_copy / total_cars_copy);
  } else {
    printf("  Average wait time: 0.000 seconds\n");
  }
  printf("  Currently waiting: %d\n", actual_waiting);
  printf("  Currently parked: %d\n", parked_copy);
  printf("\n");

  fflush(stdout);
}

// Function to simulate a car trying to access the parking lot
void *access_parking_lot(void *arg) {
  struct timespec arrival_ts;
  struct timespec parking_ts;

  // Get the current time when the car arrives
  time_t arrival_time = time(NULL);
  clock_gettime(CLOCK_MONOTONIC, &arrival_ts);

  int thread_id = (int)(long)arg;

  // Update car status to waiting
  pthread_mutex_lock(&dashboard_mutex);
  car_statuses[thread_id].car_id = thread_id;
  car_statuses[thread_id].arrival_time = arrival_time;
  car_statuses[thread_id].status = 0; // WAITING
  car_statuses[thread_id].wait_time = 0.0;
  current_waiting_count++;
  pthread_mutex_unlock(&dashboard_mutex);

  // Display dashboard after status update
  display_dashboard();

  // Simulate accessing the parking lot
  log_event("Arrived at parking lot", thread_id, arrival_time);

  // Wait for a parking space to become available
  sem_wait(&parking_semaphore);
  // Get the current time when the car successfully parks
  time_t parking_time = time(NULL);
  clock_gettime(CLOCK_MONOTONIC, &parking_ts);
  double wait_time = elapsed_seconds(&arrival_ts, &parking_ts);
  // Update total cars and total wait time
  pthread_mutex_lock(&mutex);
  total_cars++;
  total_wait_time += wait_time;
  pthread_mutex_unlock(&mutex);

  // Update car status to parked and assign parking space
  pthread_mutex_lock(&dashboard_mutex);
  car_statuses[thread_id].wait_time = wait_time;
  car_statuses[thread_id].status = 1; // PARKED
  current_waiting_count--;
  current_parked_count++;

  // Find and assign a free parking space
  for (int i = 0; i < N; i++) {
    if (!parking_spaces[i].occupied) {
      parking_spaces[i].occupied = 1;
      parking_spaces[i].car_id = thread_id;
      parking_spaces[i].occupied_since = parking_time;
      break;
    }
  }
  pthread_mutex_unlock(&dashboard_mutex);

  // Display dashboard after parking
  display_dashboard();

  // Prepare log message with wait time
  char log_message[100];
  snprintf(log_message, sizeof(log_message),
           "Parked successfully (waited %.2f seconds)", wait_time);
  log_event(log_message, thread_id, parking_time);

  // Simulate the car being parked for a random amount of time (1 to 5 seconds)
  sleep(rand() % 5 + 1);

  time_t departure_time = time(NULL);

  // Update car status to departed and free parking space
  pthread_mutex_lock(&dashboard_mutex);
  car_statuses[thread_id].status = 2; // DEPARTED
  current_parked_count--;

  // Free the parking space
  for (int i = 0; i < N; i++) {
    if (parking_spaces[i].car_id == thread_id) {
      parking_spaces[i].occupied = 0;
      parking_spaces[i].car_id = -1;
      parking_spaces[i].occupied_since = 0;
      break;
    }
  }
  pthread_mutex_unlock(&dashboard_mutex);

  // Display dashboard after departure
  display_dashboard();

  // Simulate the car leaving the parking lot
  log_event("Leaving parking lot", thread_id, departure_time);

  // Free up the parking space
  sem_post(&parking_semaphore);

  return NULL;
}

// Shared log
// Format:
// [Fri Mar 21 13:38:46 2025] Car 0: Arrived at parking lot
// [Fri Mar 21 13:38:46 2025] Car 0: Parked successfully (waited 0.00 seconds)
// [Fri Mar 21 13:38:48 2025] Car 0: Leaving parking lot
void log_event(const char *event, int thread_id, time_t event_time) {
  pthread_mutex_lock(&mutex);
  printf("[%s] Car %d: %s\n", ctime(&event_time), thread_id, event);
  pthread_mutex_unlock(&mutex);
}

int main() {
  srand((unsigned int)time(NULL) ^ (unsigned int)getpid());

  // Initialize the semaphore with N parking spaces
  sem_init(&parking_semaphore, 0, N);

  // Initialize the mutex
  if (pthread_mutex_init(&mutex, NULL) != 0) {
    perror("mutex init failed");
    return 1;
  }

  // Initialize dashboard mutex
  if (pthread_mutex_init(&dashboard_mutex, NULL) != 0) {
    perror("dashboard mutex init failed");
    return 1;
  }

  // Initialize parking spaces
  for (int i = 0; i < N; i++) {
    parking_spaces[i].occupied = 0;
    parking_spaces[i].car_id = -1;
    parking_spaces[i].occupied_since = 0;
  }

  // Initialize car statuses
  for (int i = 0; i < NUM_THREADS; i++) {
    car_statuses[i].car_id = -1;
    car_statuses[i].arrival_time = 0;
    car_statuses[i].wait_time = 0.0;
    car_statuses[i].status = -1; // Not yet started
  }

  current_parked_count = 0;
  current_waiting_count = 0;

  // Simulate 10 cars (threads) trying to access the parking lot with N spaces.
  // Create all threads first so they run in parallel
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_create(&threads_ids[i], NULL, access_parking_lot, (void *)(long)i);
  }

  // Then wait for all threads to finish
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads_ids[i], NULL);
  }

  // Final dashboard display
  display_dashboard();

  // Destroy the semaphore
  sem_destroy(&parking_semaphore);

  printf("All threads have finished accessing the parking lot.\n");
  printf("Total cars parked: %d\n", total_cars);
  if (total_cars > 0) {
    printf("Average wait time: %.3f seconds\n", total_wait_time / total_cars);
  } else {
    printf("Average wait time: 0.000 seconds\n");
  }

  // Cleanup dashboard mutex
  pthread_mutex_destroy(&dashboard_mutex);

  return 0;
}
