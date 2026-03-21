# Parking Lot Semaphore Lab

This project demonstrates the use of POSIX semaphores and threads to solve a classic concurrency problem: managing access to a limited resource (parking spaces) among multiple concurrent entities (cars).

## Core Concepts

### 1. Semaphores

A semaphore is a synchronization primitive that controls access to a common resource by multiple threads/processes. In this lab, we use an **unnamed POSIX semaphore** (`sem_t`) to represent the available parking spaces.

- **`sem_init(&parking_semaphore, 0, N)`**: Initializes the semaphore with value `N` (number of parking spaces)
- **`sem_wait(&parking_semaphore)`**: Decrements the semaphore. If the value is 0, the thread blocks until a space becomes available
- **`sem_post(&parking_semaphore)`**: Increments the semaphore, signaling that a space has been freed
- **`sem_destroy(&parking_semaphore)`**: Cleans up the semaphore resources

The semaphore ensures that no more than `N` cars can park simultaneously, implementing the **bounded resource** pattern.

### 2. Mutexes (Mutual Exclusion)

A mutex (`pthread_mutex_t`) protects shared data from race conditions when multiple threads access it concurrently.

- **`pthread_mutex_init(&mutex, NULL)`**: Initializes the mutex
- **`pthread_mutex_lock(&mutex)`**: Acquires the lock (blocks if another thread holds it)
- **`pthread_mutex_unlock(&mutex)`**: Releases the lock
- **`pthread_mutex_destroy(&mutex)`**: Destroys the mutex

In this code, the mutex protects `total_cars` and `total_wait_time` from concurrent updates.

### 3. Threads

Each car is modeled as a POSIX thread (`pthread_t`) that executes concurrently:

- **`pthread_create()`**: Creates a new thread
- **`pthread_join()`**: Waits for a thread to complete
- Threads run the `access_parking_lot()` function, simulating a car's lifecycle

### 4. The Producer-Consumer Pattern

This is a variation of the bounded buffer (producer-consumer) problem:

- **Producers**: Cars leaving and freeing up spaces (produce an available space)
- **Consumers**: Cars arriving and trying to park (consume a parking space)
- The semaphore acts as the counter for available slots

### 5. Race Conditions and Critical Sections

Without proper synchronization:

- Multiple threads could update `total_cars` and `total_wait_time` simultaneously, causing lost updates
- The semaphore count could become inconsistent if not atomic
- The mutex ensures that only one thread at a time can modify shared statistics

## How It Works

1. **Initialization**: The semaphore is set to `N` (parking spaces), mutex is initialized
2. **Thread Creation**: 10 car threads are created and start running concurrently
3. **Arrival**: Each car records its arrival time and waits on the semaphore
4. **Parking**: When a space becomes available, the car:
   - Calculates wait time
   - Updates shared statistics (under mutex protection)
   - Sleeps for 1-5 seconds (simulating parking duration)
5. **Departure**: The car leaves and posts the semaphore to free the space
6. **Cleanup**: After all threads complete, semaphore and mutex are destroyed
7. **Statistics**: Total cars parked and average wait time are displayed

## Real-Time Dashboard

The project now includes a real-time terminal-based dashboard that provides live monitoring of the parking lot system. The dashboard displays:

- **Parking Spaces**: Visual representation of all parking spots showing which are occupied and which car is parked there
- **Car Status Table**: Real-time status of all cars (WAITING, PARKED, DEPARTED) with wait times and arrival timestamps
- **Statistics**: Current counts of waiting and parked cars, plus cumulative statistics

The dashboard updates automatically whenever a car arrives, parks, or departs, giving you a complete view of the system's state as it evolves.

### Dashboard Features

1. **Visual Parking Map**: Shows all N parking spaces with car IDs
2. **Live Car Status**: Track each car's current state and wait time
3. **Real-time Counts**: See how many cars are waiting vs parked at any moment
4. **Event Logging**: All significant events are logged with timestamps

The dashboard uses a separate mutex (`dashboard_mutex`) to ensure thread-safe updates to the display data structures without interfering with the main synchronization logic.

## Build and Run with Docker

**Why Docker?** This project uses POSIX unnamed semaphores (`sem_t`, `sem_init`, `sem_wait`, `sem_post`, `sem_destroy`). These synchronization primitives are part of the POSIX standard and work natively on Linux. However, on macOS, Apple has deprecated these APIs in their system headers, which means:

- The functions may generate compiler warnings or errors
- They might not work correctly or consistently
- Future macOS versions may remove support entirely

To ensure the code runs reliably regardless of the host operating system, we use Docker to provide a consistent Linux environment. Docker allows us to:

1. **Guarantee Compatibility**: The code runs in a Debian Linux container where POSIX semaphores are fully supported
2. **Avoid Platform Issues**: No need to worry about macOS deprecation warnings or Linux vs. macOS differences
3. **Ensure Consistency**: Every developer gets the same environment, eliminating "it works on my machine" problems
4. **Simplify Setup**: No need to install additional libraries or configure the system differently

The Dockerfile uses `debian:bookworm-slim` as the base image and installs the necessary build tools (`build-essential`) to compile the C program with pthread support.

```bash
# Build the Docker image
docker build -t cc7-semaphore-lab .

# Run the container (automatically executes ./main)
docker run --rm cc7-semaphore-lab
```

### Optional: Rebuild from Scratch

If you need to rebuild without using cached layers:

```bash
docker build --no-cache -t cc7-semaphore-lab .
docker run --rm cc7-semaphore-lab
```

## Expected Output

The program displays a real-time dashboard that updates as cars arrive, park, and depart. Here's an example of what you'll see:

```
=== PARKING LOT REAL-TIME DASHBOARD ===

PARKING SPACES (2/5 occupied):
+-----+-----+-----+-----+-----+
|  0  |     |  2  |     |     |
+-----+-----+-----+-----+-----+
  [0]  [1]  [2]  [3]  [4]

CAR STATUS:
+------+-----------+-----------+----------------+
| Car  | Status    | Wait Time | Arrival Time   |
+------+-----------+-----------+----------------+
| 0    | PARKED    | 0.00s     | Fri Mar 21...  |
| 1    | WAITING   | --        | Fri Mar 21...  |
| 2    | PARKED    | 1.23s     | Fri Mar 21...  |
| 3    | DEPARTED  | 0.45s     | Fri Mar 21...  |
| 4    | WAITING   | --        | Fri Mar 21...  |
| 5    | PENDING   | --        | --              |
+------+-----------+-----------+----------------+

STATISTICS:
  Total cars processed: 3
  Average wait time: 0.560 seconds
  Currently waiting: 2
  Currently parked: 2

[Fri Mar 21 13:38:46 2025] Car 0: Arrived at parking lot
[Fri Mar 21 13:38:46 2025] Car 2: Arrived at parking lot
[Fri Mar 21 13:38:46 2025] Car 0: Parked successfully (waited 0.00 seconds)
...
All threads have finished accessing the parking lot.
Total cars parked: 10
Average wait time: 0.XXX seconds
```

## Further Reading

For a comprehensive guide to POSIX thread functions in C, including mutexes, condition variables, and thread management, see:
https://www.geeksforgeeks.org/c/thread-functions-in-c-c/

## Configuration

You can modify these constants in `main.h`:

- `NUM_THREADS`: Number of cars (threads) to simulate
- `N`: Number of parking spaces (semaphore initial value)

You can also compile with custom values from the command line using the build script:

```bash
./build.sh <num_cars> <num_parking_spaces>
./build.sh <num_cars> <num_parking_spaces> --run
```

Examples:

```bash
./build.sh 10 5
./build.sh 20 8 --run
```

## Files

- `main.c`: Main program and thread functions
- `main.h`: Header with constants and declarations
- `Dockerfile`: Container build configuration
