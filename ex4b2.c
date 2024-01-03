/*
======================================================================
File: ex4b2.c
======================================================================
Writen by : Rotem Kashani, ID = 209073352
Mahmood Jazmawy, ID = 211345277

* This is the "prime number creators" program.
* when they are ran they jump onto the queue the filler created.
* when all 3 creators are up and running they receive a signal to start drawing random
* prime number and sending them to the Filler.
* if the number the process draws enters the array im the Filler it adds one
* to a counter. after the Filler is finished and sent the signal to the process here
* to finish this process prints how much numbers it managed to insert in the array.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <math.h>

#define SERVER_ID 4
struct msg_struct {
    long _type;
    int _id;
    int _num;
};

const int FINISH = -1, FOUND = 1;

//-------------------prototype-------

//fifo check if good to continue
void check_mesg_queue(int creator_id);

bool is_prime(int num);

//making random and sending in named pipe
void create_send_random(int creator_id);

int msg_id;

//-----------------main----------
int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("worng number of arguments");
        return EXIT_FAILURE;
    }


    int creator_id = atoi(argv[1]);
    srand(creator_id);
    key_t key;
    if ((key = ftok(".", '4')) == -1) {
        perror("ftok() failed");
        exit(EXIT_FAILURE);
    }

    if ((msg_id = msgget(key, 0)) == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }


    // check pipe
    check_mesg_queue(creator_id);

    create_send_random(creator_id);

    return EXIT_SUCCESS;
}


/**
 * This function sends the filler process it's if
 * and then waits for the OK to continue running
 */
void check_mesg_queue(int creator_id) {
 
    struct msg_struct send_msg, rcv_msg;
    send_msg._type = SERVER_ID;
    send_msg._id = creator_id;
    send_msg._num = 1;
    //checking if its ok to continue
    if (msgsnd(msg_id, &send_msg, sizeof(int) * 2, 0) == -1) {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }

    if (msgrcv(msg_id, &rcv_msg, sizeof(int) * 2, creator_id, 0) == -1) {
        perror("msgrcv failed");
        exit(EXIT_FAILURE);
    }

    if (rcv_msg._num != 1) {
        perror("You may not continue");
        exit(EXIT_FAILURE);
    }

}

//----------------------
/**
 * This function creates the random numbers and sends them
 * to the filler process
 */
void create_send_random(int creator_id) {
    int found_times = 0;
    struct msg_struct server_msg, ret_msg;

    ret_msg._type = SERVER_ID;
    ret_msg._id = creator_id;
    while (1) {
        ret_msg._num = rand() % ((int) (pow(10, 6)) - 1) + 2; // draw random number
        if (is_prime(ret_msg._num)) {
            if (msgsnd(msg_id, &ret_msg, sizeof(int) * 2, 0) == -1) {
                perror("msgsnd failed");
                exit(EXIT_FAILURE);
            }

            if (msgrcv(msg_id, &server_msg, sizeof(int) * 2, creator_id, 0) == -1) {
                perror("msgrcv failed");
                exit(EXIT_FAILURE);
            }

            // if it was added add one to counter
            if (server_msg._num == FOUND) {
                found_times++;
            } else if (server_msg._num == FINISH) {
                printf("process id: %d. it has entered: %d to the array\n", creator_id, found_times);
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
