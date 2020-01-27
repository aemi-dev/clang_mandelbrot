#include <stdio.h>
#include <stdlib.h>

#define NB_NUMBERS 1000*1000*1000

int main(int argc, char ** argv)
{
    int i;
    int max_value = 0;
    unsigned int rand_state;

    for(i=0;i<NB_NUMBERS;i++)
    {
        int random_value = rand_r(&rand_state);
        if (random_value > max_value)
            max_value = random_value;
    }

    printf("My final max value is : %d .\n", max_value);

    exit(EXIT_SUCCESS);
}
