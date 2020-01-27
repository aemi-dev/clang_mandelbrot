#include <stdio.h>
#include <stdlib.h>

#define NB_NUMBERS 1000*1000*1000

#include <pthread.h>

#define NB_THREADS 512

typedef struct thread_data {
    int nb_tests;
    pthread_mutex_t * p_lock_max_value;
    int * p_max_value;
} thread_data_t;

void * testing_max_rand_numbers(void * arg)
{
    thread_data_t input = * (thread_data_t *)arg;

    int i;
    int local_max_value = 0;
    unsigned int rand_state;

    for(i=0;i<input.nb_tests;i++)
    {
        int random_value = rand_r(&rand_state);
        if (random_value > local_max_value)
            local_max_value = random_value;
    }

    pthread_mutex_lock(input.p_lock_max_value);
    if (local_max_value > *(input.p_max_value))
        *(input.p_max_value) = local_max_value;
    pthread_mutex_unlock(input.p_lock_max_value);
    
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char ** argv)
{
    int i;
    int max_value = 0;
    pthread_mutex_t lock_max_value = PTHREAD_MUTEX_INITIALIZER;
    thread_data_t thread_data_array[NB_THREADS];
    pthread_t threads[NB_THREADS];

    /* launch threads */
    for(i=0;i<NB_THREADS;i++)
    {
        if (i < NB_THREADS - 1)
            thread_data_array[i].nb_tests = NB_NUMBERS / NB_THREADS;
        /* Last threads corrects rounding gap. */
        else
            thread_data_array[i].nb_tests =
                           NB_NUMBERS - (NB_THREADS-1) * (NB_NUMBERS / NB_THREADS);
	thread_data_array[i].p_lock_max_value = &lock_max_value;
        thread_data_array[i].p_max_value = & max_value;

        pthread_create(&threads[i], NULL, testing_max_rand_numbers,
                       (void *) &thread_data_array[i]);
    }

    /* wait for threads */
    for(i=0;i<NB_THREADS;i++)
    {
        pthread_join(threads[i],NULL);
    }

    printf("My final max value is : %d .\n", max_value);

    exit(EXIT_SUCCESS);
}
