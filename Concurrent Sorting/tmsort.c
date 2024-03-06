#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#define tty_printf(...) (isatty(1) && isatty(0) ? printf(__VA_ARGS__) : 0)

#ifndef SHUSH
#define log(...) (fprintf(stderr, __VA_ARGS__))
#else
#define log(...)
#endif

// Global variable
int thread_count = 1; // # of threads for sorting

typedef struct {
  long *nums;
  long *target;
  int from;
  int to;
  int thread_count;
} merge_ctx;

// Forward dec of the aux func for merge sort
void merge_sort_aux(long nums[], int from, int to, long target[], int thread_count);

// Computes time diff (seconds)
double time_in_secs(const struct timeval *begin, const struct timeval *end) {
  long s = end->tv_sec - begin->tv_sec;
  long ms = end->tv_usec - begin->tv_usec;
  return s + ms * 1e-6;
}

// Print array of longs
void print_long_array(const long *array, int count) {
  for (int i = 0; i < count; ++i) {
    printf("%ld\n", array[i]);
  }
}

// Merges 2 sorted halves of an array
void merge(long nums[], int from, int mid, int to, long target[]) {
  // Merge 2 sorted sub-arrays into 1
  int left = from, right = mid, i = from;
  for (; left < mid && right < to; i++) {
    if (nums[left] <= nums[right]) {
      target[i] = nums[left++];
    } else {
      target[i] = nums[right++];
    }
  }
  // Copy remaining from L || R sub-array
  if (left < mid) {
    memmove(&target[i], &nums[left], (mid - left) * sizeof(long));
  } else if (right < to) {
    memmove(&target[i], &nums[right], (to - right) * sizeof(long));
  }
}

// Parallel merge sort thread 
void *merge_sort_aux_threaded(void *ref) {
  merge_ctx *ctx = (merge_ctx *)ref;
  merge_sort_aux(ctx->nums, ctx->from, ctx->to, ctx->target, ctx->thread_count);
  pthread_exit(NULL);
}

// Perform merge sort (recursive)
void merge_sort_aux(long nums[], int from, int to, long target[], int thread_count) {
  if (to - from <= 1) return;

  int mid = (from + to) / 2;
  if (thread_count <= 1) {
    // Ff thread count low, sequential merge sort 
    merge_sort_aux(target, from, mid, nums, 1);
    merge_sort_aux(target, mid, to, nums, 1);
    goto merge;
  }

  // Parallel merge sort
  pthread_t thread;
  merge_ctx args = {target, nums, from, mid, thread_count / 2};
  pthread_create(&thread, NULL, merge_sort_aux_threaded, &args);
  merge_sort_aux(target, mid, to, nums, (thread_count + 1) / 2);
  void *ret;
  pthread_join(thread, &ret);

merge:
  merge(nums, from, mid, to, target);
}

// Function to sort an array using merge sort and return the sorted array
long *merge_sort(long nums[], int count) {
  long *result = calloc(count, sizeof(long));
  assert(result != NULL);
  memmove(result, nums, count * sizeof(long));
  merge_sort_aux(nums, 0, count, result, thread_count);
  return result;
}

// Reads input array from CLI args
int allocate_load_array(int argc, char **argv, long **array) {
  assert(argc > 1);
  int count = atoi(argv[1]);
  *array = calloc(count, sizeof(long));
  assert(*array != NULL);
  long element;
  tty_printf("Enter %d elements, separated by whitespace\n", count);
  for (int i = 0; i < count && scanf("%ld", &element) != EOF; i++) {
    (*array)[i] = element;
  }
  return count;
}

// To execute merge sort
int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <n>\n", argv[0]);
    return 1;
  }
  struct timeval begin, end;

  // Config thread count based on envir var
  if (getenv("MSORT_THREADS") != NULL)
    thread_count = atoi(getenv("MSORT_THREADS"));

  log("Running with %d thread(s). Reading input.\n", thread_count);
  long *array = NULL;
  int count = allocate_load_array(argc, argv, &array);
  gettimeofday(&begin, 0);
  long *result = merge_sort(array, count);
  gettimeofday(&end, 0);
  log("Sorting completed in %f seconds.\n", time_in_secs(&begin, &end));
  gettimeofday(&begin, 0);
  print_long_array(result, count);
  gettimeofday(&end, 0);
  log("Array printed in %f seconds.\n", time_in_secs(&begin, &end));
  
  free(array);
  free(result);
  return 0;
}
