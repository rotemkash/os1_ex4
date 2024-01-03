/*
======================================================================
File: ex4b1.c
======================================================================
Writen by : Rotem Kashani, ID = 209073352
Mahmood Jazmawy, ID = 211345277

* This if the "Filler" process. initializes an array with 100 cells.
* it loops a 100 times getting data through message queue from 3 other processes
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
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>


//struct to hold the message
struct msg_struct {
    long _type;
    int _id;
    int _num;
};

#define ARR_SIZE 100

//-------------------prototype-------
bool check_all_running();

void fill_array(int arr[]);

void send_ok();

void catch_sigint(int sig_num);

void free_queue();

//------------------- const --------
const int NUM_OF_GEN = 3, CAN_RUN = 1, FINISH = -1;

int msg_id;

//---------------MAIN---------------
int main() {

    int arr[ARR_SIZE];
    key_t key;

    signal(SIGINT, catch_sigint);

    if ((key = ftok(".", '4')) == -1) {
        perror("ftok() failed");
        exit(EXIT_FAILURE);
    }
    if ((msg_id = msgget(key, IPC_CREAT | IPC_EXCL | 0600)) == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }

    // check if all process are up and running
    if (check_all_running()) {
        // send them the OK to start sending numbers
        send_ok();
    } else {
        perror("not all processes are running");
        exit(EXIT_FAILURE);
    }

    // fill the array with prime numbers
    fill_array(arr);

    return EXIT_SUCCESS;
}

/**
 * This function sends the OK to the processes
 */
void send_ok() {
    struct msg_struct msg;
    msg._id = 0;
    msg._num = CAN_RUN;
    for (int i = 1; i <= NUM_OF_GEN; ++i) {
        msg._type = i;
        if (msgsnd(msg_id, &msg, sizeof(int) * 2, 0) == -1) {
            perror("msgsnd failed");
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * check if all process are running
 */
bool check_all_running() {
    struct msg_struct msg;
    bool is_running = true;
    //waiting for 3 "OK's" and then sending the gens ok to run the prime check
    for (int i = 1; i <= NUM_OF_GEN; i++) {

        if (msgrcv(msg_id, &msg, sizeof(int) * 2, 4, 0) == -1) {
            perror("msgrcv failed");
            exit(EXIT_FAILURE);
        }

        if (msg._num == 1) {
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
void fill_array(int arr[]) {
    //getting prime numbers from gens and checking in the arr f exist,and how many times
    int found = 0, curr_cell = 0;
    struct msg_struct client_msg, ret_msg;
    ret_msg._id = 0;

    for (int i = 0; i < ARR_SIZE; ++i) {
        if (msgrcv(msg_id, &client_msg, sizeof(int) * 2, 4, 0) == -1) {
            perror("msgrcv failed");
            exit(EXIT_FAILURE);
        }

        if (curr_cell == 0) { // if it's the first cell
            arr[curr_cell++] = client_msg._num;
            found = 1;
        } else { // if there's already values in the array
            if (arr[curr_cell - 1] <= client_msg._num) {
                arr[curr_cell++] = client_msg._num;
                found = 1;
            } else {
                found = 0;
            }
        }

        ret_msg._type = client_msg._id;
        ret_msg._num = found;
        if(i != ARR_SIZE - 1){
            if (msgsnd(msg_id, &ret_msg, sizeof(int)*2, 0) == -1){
                perror("msgsnd failed");
                exit(EXIT_FAILURE);
            }
        }
    }


    // print the data we were asked to show
    printf("array size: %d. min value: %d. max value: %d\n", curr_cell, arr[0], arr[curr_cell - 1]);

    ret_msg._id = 0;
    ret_msg._num = -1;
    // send the processes a msg to indicate they can finish and print
    for (int i = 1; i <= NUM_OF_GEN; ++i) {
        ret_msg._type = i;

        if (msgsnd(msg_id, &ret_msg, sizeof(int) * 2, 0) == -1) {
            perror("msgsnd failed");
            exit(EXIT_FAILURE);
        }
        if (i < NUM_OF_GEN) {
            if (msgrcv(msg_id, &client_msg, sizeof(int) * 2, 4, 0) == -1) {
                perror("msgrcv failed");
                exit(EXIT_FAILURE);
            }
        }

    }
    free_queue();
}


/*
 * Catch SIGINT and free the queue
 */
void catch_sigint(int sig_num) {
    free_queue();
}

/**
 * free the message queue
 */
void free_queue() {
    if (msgctl(msg_id, IPC_RMID, NULL) == -1) {
        perror("msgctl failed");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
