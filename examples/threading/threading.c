#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

struct thread_data {
    pthread_mutex_t* mutex;
    int wait_to_obtain_ms;
    int wait_to_release_ms;
};

void* threadfunc(void* thread_param) {
    struct thread_data* thread_func_args = (struct thread_data*) thread_param;

    useconds_t wait_to_obtain_us = thread_func_args->wait_to_obtain_ms * 1000;
    useconds_t wait_to_release_us = thread_func_args->wait_to_release_ms * 1000;

    // Sleep before attempting to lock the mutex
    usleep(wait_to_obtain_us);

    // Try to obtain the mutex
    pthread_mutex_lock(thread_func_args->mutex);
    DEBUG_LOG("Mutex obtained");

    // Sleep while holding the mutex
    usleep(wait_to_release_us);

    // Release the mutex
    pthread_mutex_unlock(thread_func_args->mutex);
    DEBUG_LOG("Mutex released");

    free(thread_func_args);  // Free the memory allocated for thread data
    return NULL;
}

bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms) {
    struct thread_data* data = (struct thread_data*)malloc(sizeof(struct thread_data));
    if (data == NULL) {
        ERROR_LOG("Memory allocation failed for thread data");
        return false;
    }

    data->mutex = mutex;
    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms;

    // Create the thread and pass the data structure to it
    if (pthread_create(thread, NULL, threadfunc, (void*)data) != 0) {
        ERROR_LOG("Failed to create thread");
        free(data);
        return false;
    }

    return true;
}

