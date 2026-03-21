#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 || $# -gt 3 ]]; then
  echo "Usage: $0 <num_cars> <num_parking_spaces> [--run]"
  echo "Example: $0 10 5"
  echo "Example: $0 20 8 --run"
  exit 1
fi

NUM_CARS="$1"
PARKING_SPACES="$2"
RUN_AFTER_BUILD="${3:-}"

if ! [[ "$NUM_CARS" =~ ^[1-9][0-9]*$ ]]; then
  echo "Error: <num_cars> must be a positive integer."
  exit 1
fi

if ! [[ "$PARKING_SPACES" =~ ^[1-9][0-9]*$ ]]; then
  echo "Error: <num_parking_spaces> must be a positive integer."
  exit 1
fi

if [[ -n "$RUN_AFTER_BUILD" && "$RUN_AFTER_BUILD" != "--run" ]]; then
  echo "Error: optional third argument must be --run"
  exit 1
fi

echo "Compiling with NUM_THREADS=$NUM_CARS and N=$PARKING_SPACES ..."
gcc -Wall -Wextra -pthread -DNUM_THREADS="$NUM_CARS" -DN="$PARKING_SPACES" main.c -o main

echo "Build completed: ./main"

if [[ "$RUN_AFTER_BUILD" == "--run" ]]; then
  echo "Running ./main ..."
  ./main
fi
