# Parking Lot Semaphore Lab (Docker)

This project uses POSIX unnamed semaphores (`sem_init`, `sem_wait`, `sem_post`, `sem_destroy`).
On macOS these APIs are marked deprecated by Apple headers, but they are fully supported on Linux.

## Build and Run with Docker

```bash
docker build -t cc7-semaphore-lab .
docker run --rm cc7-semaphore-lab
```

## Optional: Rebuild and run quickly

```bash
docker build --no-cache -t cc7-semaphore-lab .
docker run --rm cc7-semaphore-lab
```