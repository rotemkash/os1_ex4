/*
======================================================================
File: ex4a2.c
======================================================================
Writen by : Rotem Kashani, ID = 209073352
Mahmood Jazmawy, ID = 211345277

* This is the "prime number creators" program.
* when they are ran they create a pipe with the filler process.
* when all 3 creators are up and running they receive a signal to start drawing random
* prime number and sending them to the Filler.
* if the number the process draws enters the array im the Filler it adds one
* to a counter. after the Filler is finished and sent the signal to the process here
* to finish this process prints how much numbers it managed to insert in the array.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

const int FINISH = -1, FOUND = 1, CAN_RUN = 1;

//-------------------prototype-------

//fifo check if good to continue
void check_fifo(FILE *fdw, FILE *fdr, int id);

bool is_prime(int num);

//making random and sending in named pipe
void create_send_random(FILE *fdw, FILE *fdr, int id);

//-----------------main----------
int main(int argc, char *argv[]) {
    FILE *fdr = NULL, *fdw = NULL;
    char this_fifo[6] = "fifo";

    if (argc != 3) {
        printf("worng number of arguments");
        return EXIT_FAILURE;
    }
    char *filler_fifo = argv[1];
    int p_id = atoi(argv[2]);
    srand(p_id);

    if (!(fdw = fopen(filler_fifo, "w"))) {
        perror("cannot create fifo file for w");
        exit(EXIT_FAILURE);
    }

    this_fifo[4] = *argv[2];
    if (!(fdr = fopen(this_fifo, "r"))) {
        perror("cannot create fifo file for r");
        exit(EXIT_FAILURE);
    }
    // check pipe
    check_fifo(fdw, fdr, p_id);
    create_send_random(fdw, fdr, p_id);

    fclose(fdr);
    fclose(fdw);
    return EXIT_SUCCESS;
}


/**
 * This function sends the filler process it's if
 * and then waits for the OK to continue running
 */
void check_fifo(FILE *fdw, FILE *fdr, int id) {
    //checking if its ok to continue
    int checker = 0;
    fprintf(fdw, "%d ", id);
    fflush(fdw);
    fscanf(fdr, "%d", &checker);

    if (checker != CAN_RUN) {
        perror("You may not continue");
        exit(EXIT_FAILURE);
    }
}

//----------------------
/**
 * This function creates the random numbers and sends them
 * to the filler process
 */
void create_send_random(FILE *fdw, FILE *fdr, int id) {
    int num, count_exist_num = 0, found_times = 0;

    while (1) {
        num = rand() % ((int) (pow(10, 6)) - 1) + 2; // draw random number

        if (is_prime(num)) {
            fprintf(fdw, "%d %d ", num, id);// send num to filler
            fflush(fdw);
            fscanf(fdr, "%d", &count_exist_num);//get result from filler

            // if it was added add one to counter
            if (count_exist_num == FOUND) {
                found_times++;
            } else if (count_exist_num == FINISH) {
                printf("process num: %d. it has entered: %d to the array\n", id, found_times);
                exit(EXIT_SUCCESS);
            }
        }
    }
}


/**
 * This function checks if a number is prime or not
 */
bool is_prime(int num) {
    int mid = num / 2;
    if (num < 2) {
        return false;
    }
    for (int i = 2; i <= mid; i++) {
        if (num % i == 0) {
            return false;
        }
    }
    return true;
}
