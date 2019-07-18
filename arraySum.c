/* 
 * File:   main.c
 * Author: douglas
 *
 * Created on 2 de Julho de 2019, 20:02
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
#include <getopt.h>
#include <sched.h>
#include <sys/time.h>
#include "tinySTM/include/stm.h"
#include "tinySTM/include/mod_mem.h"
#include "tinySTM/include/mod_stats.h"

/**
 * STM Utils
 */
#define TM_STARTUP()     \
        stm_init();       \
        mod_mem_init(0);  \
        mod_stats_init() 

#define TM_START()  \
        sigjmp_buf * checkPoint = stm_start((stm_tx_attr_t) 0); \
        sigsetjmp(*checkPoint, 0); 

#define TM_LOAD(addr)                      stm_load((stm_word_t *)addr)
#define TM_LOAD_I(addr)                    (int) TM_LOAD(addr)  
#define TM_STORE(addr, value)              stm_store((stm_word_t *)addr, (stm_word_t)value)
#define TM_STATS()     \
    unsigned long stat;     \
    if (stm_get_global_stats("global_nb_commits", &stat) != 0) \
      printf("#commits    : %lu\n", stat);   \
    if (stm_get_global_stats("global_nb_aborts", &stat) != 0) \
      printf("#aborts     : %lu\n", stat)

/**
 * Timer Utils (from Stamp Benchmark)
 */
#define TIMER_T                         struct timeval
#define TIMER_READ(time)                gettimeofday(&(time), NULL)
#define TIMER_DIFF_SECONDS(start, stop) \
    (((double)(stop.tv_sec)  + (double)(stop.tv_usec / 1000000.0)) - \
     ((double)(start.tv_sec) + (double)(start.tv_usec / 1000000.0)))



#define NUM_THREADS 8
#define ARRAY_SIZE 16000000 //16.000.000 
#define CHUNK ARRAY_SIZE / NUM_THREADS

/* Global variables */
int array[ARRAY_SIZE];
int threadsArgs[NUM_THREADS];

int pinCoreType;
//0 - no pin, OS default
//1 - worst pin
//2 - best pin (cache)
//3 - best pin 2 (cache and NUMA node)

const int worst_pin[] = {0, 42, 6, 36, 12, 30, 18, 24};
const int best_pin[] = {0, 1, 6, 7, 12, 13, 18, 19};
const int best_pin2[] = {0, 1, 2, 3, 4, 5, 6, 7};

pthread_t threads[NUM_THREADS];

int sumValidation = 0; //final verification value

/* Threads sum value (each 2 threads) */
int sum_0_1;
int sum_2_3;
int sum_4_5;
int sum_6_7;

void thread_pin_to_cpu(pthread_t thread, int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    const int set_affinity = pthread_setaffinity_np(thread, sizeof (cpu_set_t), &cpuset);
    if (set_affinity != 0) {
        perror("thread_pin_to_cpu_func");
        exit(EXIT_FAILURE);
    }
}

void *doSum(void *args) {
    int threadId = *((int *) args);
    if (pinCoreType == 1) { //worst pin
        thread_pin_to_cpu(pthread_self(), worst_pin[threadId]);
    } else if (pinCoreType == 2) { //best pin
        thread_pin_to_cpu(pthread_self(), best_pin[threadId]);
    } else if (pinCoreType == 3) { //best pin 2
        thread_pin_to_cpu(pthread_self(), best_pin2[threadId]);
    }

    stm_init_thread();

    int i;
    int idx = threadId * CHUNK;
    int value;
    for (i = idx; i < idx + CHUNK; i++) {
        TM_START();
        value = TM_LOAD(&array[i]);
        if (threadId == 0 || threadId == 1) {
            TM_STORE(&sum_0_1, TM_LOAD(&sum_0_1) + value);
        } else if (threadId == 2 || threadId == 3) {
            TM_STORE(&sum_2_3, TM_LOAD(&sum_2_3) + value);
        } else if (threadId == 4 || threadId == 5) {
            TM_STORE(&sum_4_5, TM_LOAD(&sum_4_5) + value);
        } else if (threadId == 6 || threadId == 7) {
            TM_STORE(&sum_6_7, TM_LOAD(&sum_6_7) + value);
        }
        stm_commit();
    }
    stm_exit_thread();
}

void initThreads() {
    for (int i = 0; i < NUM_THREADS; i++) {
        threadsArgs[i] = i;
        pthread_create(&(threads[i]), NULL, doSum, &threadsArgs[i]);
    }

    //synchronize
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}

void initializeData() {
    int i;
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = rand() % 10;
        sumValidation += array[i];
    }
}

/*
 * 
 */
int main(int argc, char** argv) {
    int opt = getopt(argc, (char**) argv, "c:");
    if (opt == EOF) {
        fprintf(stderr, "Parameter -c (type of pin to cores) is mandatory\n");
        return (EXIT_FAILURE);
    } else {
        pinCoreType = atoi(optarg);
        if (pinCoreType < 0 || pinCoreType > 3) {
            fprintf(stderr, "Parameter -c need to be 0, 1, 2 or 3\n");
            return (EXIT_FAILURE);
        }
    }

    srand(time(NULL));

    initializeData();

    TIMER_T startTime;
    TIMER_READ(startTime);

    TM_STARTUP();
    initThreads();
    TM_STATS();

    TIMER_T stopTime;
    TIMER_READ(stopTime);

    stm_exit();

    // Verify if the parallel sum is correct.
    assert(sumValidation == (sum_0_1 + sum_2_3 + sum_4_5 + sum_6_7));

    char* pinName;
    if (pinCoreType == 0) {
        pinName = "Default";
    } else if (pinCoreType == 1) {
        pinName = "Worst";
    } else if (pinCoreType == 2) {
        pinName = "Best";
    } else if (pinCoreType == 3) {
        pinName = "Best2";
    }

    printf("%f\t%s\n", TIMER_DIFF_SECONDS(startTime, stopTime), pinName);

    return (EXIT_SUCCESS);
}

