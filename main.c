#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>


#define NUM_THREADS 10
#define N 5 // Number of parking spaces
// TODO: Use command line arguments to set N

pthread_t threads_ids[NUM_THREADS];
pthread_mutex_t mutex;

sem_t parking_semaphore; // Initialize the semaphore. Will be set to N in main

// Function to simulate a car trying to access the parking lot
void *access_parking_lot(void *arg)
{
    // Get the current time when the car arrives
    time_t arrival_time = time(NULL);
    // Log its arrival timestamp
    printf("Thread %d has arrived at the parking lot at time %s", (int)(long)arg, ctime(&arrival_time));


    printf("Thread %d has arrived at the parking lot.\n", (int)(long)arg);
    int thread_id = (int)(long)arg;
    // Simulate accessing the parking lot
    printf("Thread %d is accessing the parking lot.\n", thread_id);
    return NULL;
}

// Function to simulate a car parking
void *car_parking(void *arg);

// Function to simulate a car leaving the parking lot and freeing up a space
void *car_departure(void *arg);



int main()
{
    time_t current_time = time(NULL);

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

    return 0;
}