#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// TODO: Use command line arguments to set N

pthread_t threads_ids[NUM_THREADS];
pthread_mutex_t mutex;

sem_t parking_semaphore; // Initialize the semaphore. Will be set to N in main

int total_cars = 0; // Total number of cars that have parked.
double total_wait_time = 0.0; // Total wait time for all cars.

static double elapsed_seconds(const struct timespec *start, const struct timespec *end)
{
    return (double)(end->tv_sec - start->tv_sec) +
           (double)(end->tv_nsec - start->tv_nsec) / 1000000000.0;
}

// Function to simulate a car trying to access the parking lot
void *access_parking_lot(void *arg)
{
    struct timespec arrival_ts;
    struct timespec parking_ts;

    // Get the current time when the car arrives
    time_t arrival_time = time(NULL);
    clock_gettime(CLOCK_MONOTONIC, &arrival_ts);

    int thread_id = (int)(long)arg;
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

    // Prepare log message with wait time
    char log_message[100];
    snprintf(log_message, sizeof(log_message), "Parked successfully (waited %.2f seconds)", wait_time);
    log_event(log_message, thread_id, parking_time);

    // Simulate the car being parked for a random amount of time (1 to 5 seconds)
    sleep(rand() % 5 + 1);

    time_t departure_time = time(NULL);
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


int main()
{
    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());

    // Initialize the semaphore with N parking spaces
    sem_init(&parking_semaphore, 0, N);

    // Initialize the mutex
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("mutex init failed");
        return 1;
    }

    // Simulate 10 cars (threads) trying to access the parking lot with N spaces.
    // Create all threads first so they run in parallel
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&threads_ids[i], NULL, access_parking_lot, (void *)(long)i);
    }

    // Then wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads_ids[i], NULL);
    }

    // Destroy the semaphore
    sem_destroy(&parking_semaphore);

    printf("All threads have finished accessing the parking lot.\n");
    printf("Total cars parked: %d\n", total_cars);
    if (total_cars > 0)
    {
        printf("Average wait time: %.3f seconds\n", total_wait_time / total_cars);
    }
    else
    {
        printf("Average wait time: 0.000 seconds\n");
    }

    return 0;
}