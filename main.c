#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>


#define NUM_THREADS 10
#define N 5 // Number of parking spaces
// TODO: Use command line arguments to set N

pthread_t threads_ids[NUM_THREADS];
pthread_mutex_t mutex;

sem_t parking_semaphore = {0}; // Initialize the semaphore with 0, will set to N in main

void *access_parking_lot(void *arg)
{
    int thread_id = (int)(long)arg;
    // Simulate accessing the parking lot
    printf("Thread %d is accessing the parking lot.\n", thread_id);
    return NULL;
}

int main()
{
    // Initialize the semaphore with N parking spaces
    sem_init(&parking_semaphore, 0, N);

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