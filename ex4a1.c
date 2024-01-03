/*
======================================================================
File: ex4a1.c
======================================================================
Writen by : Rotem Kashani, ID = 209073352
Mahmood Jazmawy, ID = 211345277

* This if the "Filler" process. initializes an array with 100 cells.
* it loops a 100 times getting data through named-pipe from 3 other processes
* if the number sent by the process is bigger/equal to the last number in the array
* it inserts that number in the array and send the matching process 1.
* if the number is not larger/equal it send 0.
* after 100 iterations, whether the array is full or not, it prints the data we
* were asked to show (array size, min value, max value).
* then it sends -1 to the process indicating to finish and printing
* how many number each process has entered o the array.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <errno.h>

#define ARR_SIZE 100

//-------------------prototype-------
void make_fifo(const char *fifo_name);

bool check_all_running(FILE *fdr);

void open_fifo(FILE **fifo_file, const char *argv);

void fill_array(int arr[], FILE *fdw1, FILE *fdw2, FILE *fdw3, FILE *fifo_r);

void send_ok(FILE *fdw_num);

void finish_processes(FILE *fdw1, FILE *fdw2, FILE *fdw3, FILE *fifo_r);

//------------------- const --------
const int NUM_OF_GEN = 3, CAN_RUN = 1, FINISH = -1;


//---------------MAIN---------------
int main(int argc, const char *argv[]) {

    // named pipes for all processes
    FILE *fdr0, *fdw1, *fdw2, *fdw3;
    int arr[ARR_SIZE] = {0}; // array to fill

    // mkfifo for all files
    for (int i = 1; i < 5; ++i) {
        make_fifo(argv[i]);
    }

    if (!(fdr0 = fopen(argv[1], "r"))) {
        perror("cannot open fifo file for r");
        exit(EXIT_FAILURE);
    }

    open_fifo(&fdw1, argv[2]); // open fifo1
    open_fifo(&fdw2, argv[3]); // open fifo2
    open_fifo(&fdw3, argv[4]); // open fifo3

    // check if all process are up and running
    if (check_all_running(fdr0)) {
        // send them the OK to start sending numbers
        send_ok(fdw1);
        send_ok(fdw2);
        send_ok(fdw3);
    } else {
        perror("not all processes are running");
        exit(EXIT_FAILURE);
    }

    // fill the array with prime numbers
    fill_array(arr, fdw1, fdw2, fdw3, fdr0);

    // close all pipes
    fclose(fdr0);
    fclose(fdw1);
    fclose(fdw2);
    fclose(fdw3);
    return EXIT_SUCCESS;
}

/**
 * This function sends the OK to the processes
 */
void send_ok(FILE *fdw_num) {
    fprintf(fdw_num, "%d ", CAN_RUN);
    fflush(fdw_num);
}

/**
 * Opens the pipes for writing
 */
void open_fifo(FILE **fifo_file, const char *argv) {
    if (!(*fifo_file = fopen(argv, "w"))) {
        perror("cannot open fifo file for w");
        exit(EXIT_FAILURE);
    }
}

/**
 * check if all process are running
 */
bool check_all_running(FILE *fdr) {
    int prod;
    bool is_running = true;
    //waiting for 3 "OK's" and then sending the gens ok to run the prime check
    for (int i = 0; i < NUM_OF_GEN; i++) {
        fscanf(fdr, "%d", &prod);
        if (prod == 1 || prod == 2 || prod == 3) {
            continue;
        } else {
            is_running = false;
            break;
        }
    }
    return is_running;
}


/**
 * this function receives an empty array and a pointer to the necessary pipes.
 * it reads the numbers from the 3 processes and sends them the relevant data in return.
 * then after a 100 iterations it prints the data that we were asked to show.
 * then it sends the processes a number to indicate they have to finish
 */
void fill_array(int arr[], FILE *fdw1, FILE *fdw2, FILE *fdw3, FILE *fifo_r) {
    //getting prime numbers from gens and checking in the arr f exist,and how many times
    int producer_id = 0;
    int found = 0, curr_cell = 0, prime_to_add = 0;

    for (int i = 0; i < ARR_SIZE; i++) {
        fscanf(fifo_r, "%d %d", &prime_to_add, &producer_id);

        if (curr_cell == 0) { // if it's the first cell
            arr[curr_cell++] = prime_to_add;
            found = 1;
        } else { // if there's already values in the array
            if (arr[curr_cell - 1] <= prime_to_add) {
                arr[curr_cell++] = prime_to_add;
                found = 1;
            } else {
                found = 0;
            }
        }

        if (producer_id == 1) {
            fprintf(fdw1, "%d ", found);
            fflush(fdw1);
        } else if (producer_id == 2) {
            fprintf(fdw2, "%d ", found);
            fflush(fdw2);
        } else if (producer_id == 3) {
            fprintf(fdw3, "%d ", found);
            fflush(fdw3);
        }
    }

    // print the data we were asked to show
    printf("array size: %d. min value: %d. max value: %d\n", curr_cell, arr[0], arr[curr_cell - 1]);
    finish_processes(fdw1, fdw2, fdw3, fifo_r);
}

/**
 * This function is called at the end to send the process the number
 * that indicates they have to end
 */
void finish_processes(FILE *fdw1, FILE *fdw2, FILE *fdw3, FILE *fifo_r) {
    int tmp1, tmp2;
        fscanf(fifo_r, "%d %d", &tmp1, &tmp2);
        fprintf(fdw1, "%d ", FINISH);
        fflush(fdw1);
        fscanf(fifo_r, "%d %d", &tmp1, &tmp2);
        fprintf(fdw2, "%d ", FINISH);
        fflush(fdw2);
        fscanf(fifo_r, "%d %d", &tmp1, &tmp2);
        fprintf(fdw3, "%d ", FINISH);
        fflush(fdw3);
}

//-----------------------
// make fifo
void make_fifo(const char *fifo_name) {
    if (mkfifo(fifo_name, S_IFIFO | 0644) == -1 && errno != EEXIST) {
        perror("cannot mkfifo fifo");
        exit(EXIT_FAILURE);
    }
}
