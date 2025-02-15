#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Structure to hold data specific to each thread
struct thread_data {
    pthread_mutex_t* mutex;           // Pointer to the mutex
    int wait_to_obtain_ms;            // Time in milliseconds to wait before attempting to lock
    int wait_to_release_ms;           // Time in milliseconds to hold the mutex before releasing
    bool thread_complete_success;     // Indicates whether the thread completed successfully
};

// Optional: debug logging
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

/**
 * The function executed by the thread.
 * It will wait for the specified time to obtain the mutex, then lock it, 
 * wait for another specified time, and finally release it.
 */
void* threadfunc(void* thread_param) {
    struct thread_data* thread_func_args = (struct thread_data*) thread_param;

    // Convert milliseconds to microseconds (usleep works with microseconds)
    useconds_t wait_to_obtain_us = thread_func_args->wait_to_obtain_ms * 1000;
    useconds_t wait_to_release_us = thread_func_args->wait_to_release_ms * 1000;

    // Sleep before attempting to lock the mutex
    usleep(wait_to_obtain_us);

    // Try to obtain the mutex
    if (pthread_mutex_lock(thread_func_args->mutex) != 0) {
        thread_func_args->thread_complete_success = false;
        ERROR_LOG("Failed to obtain mutex");
        return (void*)thread_func_args;
    }
    DEBUG_LOG("Mutex obtained");

    // Sleep while holding the mutex
    usleep(wait_to_release_us);

    // Release the mutex
    if (pthread_mutex_unlock(thread_func_args->mutex) != 0) {
        thread_func_args->thread_complete_success = false;
        ERROR_LOG("Failed to release mutex");
        return (void*)thread_func_args;
    }
    DEBUG_LOG("Mutex released");

    // If everything went well
    thread_func_args->thread_complete_success = true;

    return (void*)thread_func_args;
}

/**
 * Start a thread that will sleep for `wait_to_obtain_ms`, obtain a mutex,
 * hold it for `wait_to_release_ms`, and then release the mutex.
 * The thread will return a pointer to its `thread_data` structure, 
 * which is dynamically allocated and should be freed by the joiner thread.
 * The function does not block the caller and allows the creation of multiple threads.
 */
bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms) {
    // Allocate memory for the thread data structure
    struct thread_data* data = (struct thread_data*)malloc(sizeof(struct thread_data));
    if (data == NULL) {
        ERROR_LOG("Memory allocation failed for thread data");
        return false;
    }

    // Initialize the data
    data->mutex = mutex;
    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms;
    data->thread_complete_success = false; // Initially set to false

    // Create the thread and pass the data structure to it
    if (pthread_create(thread, NULL, threadfunc, (void*)data) != 0) {
        ERROR_LOG("Failed to create thread");
        free(data);
        return false;
    }

    return true;
}

/**
 * A simple function to join the thread and check the result
 */
void check_thread_status(pthread_t thread) {
    void* return_value;
    if (pthread_join(thread, &return_value) != 0) {
        ERROR_LOG("Failed to join thread");
        return;
    }

    struct thread_data* data = (struct thread_data*)return_value;
    if (data->thread_complete_success) {
        DEBUG_LOG("Thread completed successfully");
    } else {
        ERROR_LOG("Thread encountered an error");
    }

    // Free the dynamically allocated memory for thread data
    free(data);
}

int main() {
    pthread_mutex_t mutex;
    pthread_t thread1, thread2;

    // Initialize the mutex
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        ERROR_LOG("Failed to initialize mutex");
        return 1;
    }

    // Start two threads with different parameters
    if (!start_thread_obtaining_mutex(&thread1, &mutex, 500, 1000)) {
        ERROR_LOG("Failed to start thread 1");
        return 1;
    }

    if (!start_thread_obtaining_mutex(&thread2, &mutex, 1000, 1500)) {
        ERROR_LOG("Failed to start thread 2");
        return 1;
    }

    // Check the status of the threads
    check_thread_status(thread1);
    check_thread_status(thread2);

    // Destroy the mutex
    pthread_mutex_destroy(&mutex);

    return 0;
}

