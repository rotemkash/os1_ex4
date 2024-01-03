
/*
======================================================================
File: ex4c1.c
======================================================================
Writen by : Rotem Kashani, ID = 209073352
    Mahmood Jazmawy, ID = 211345277

 * This program is our server that checks for prime numbers.
 * it receives a string of number through the message queue.
 * it goes over all the number in the strings and checks for prime numbers
 * it adds all the prime number to a new string and sends it back to the client
 */

//==============-INCLUDES-===============
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

//================-DEFINE-================
#define MAX_LEN 1000

//==============-STRUCTS-================
struct txt_msg {
    long _mtype;
    char _mtxt[MAX_LEN];
};

//===============-PROTOTYPES-=============
void catch_sigint(int sig_num);

bool is_prime(int num);

//============-GLOBLAS-================
int msgid;


//==============-MAIN-=====================
int main() {
    struct txt_msg my_msg;
    key_t key;
    signal(SIGINT, catch_sigint);


    // create message queue
    if ((key = ftok(".", 'p')) == -1) {
        fprintf(stderr, "ftok() failed\n");
        exit(EXIT_FAILURE);
    }
    if ((msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0600)) == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }

    char *token;
    int tmp;
    char ret_txt[MAX_LEN];

    while (1) {
        // reset strings just in case
        for (int i = 0; i < MAX_LEN; ++i) {
            ret_txt[i] = '\0';
            my_msg._mtxt[i] = '\0';
        }

        if ((msgrcv(msgid, &my_msg, MAX_LEN, 'p', 0)) == -1) {
            perror("msgrcv failed");
            exit(EXIT_FAILURE);
        }


        /*
         * split the string around the space and check each number
         */
        token = strtok(my_msg._mtxt, " ");
        while (token != NULL) {
            tmp = atoi(token);

            /*
             * if the current token(number) is a prime.
             * add it to the existing string and add a space after it.
             */
            if (is_prime(tmp)) {
                strcat(ret_txt, token);
                if (strlen(ret_txt) < MAX_LEN) {
                    ret_txt[strlen(ret_txt)] = ' ';
                }
            }
            token = strtok(NULL, " ");
        }

        // copy the result to our struct and send it
        strcpy(my_msg._mtxt, ret_txt);
        my_msg._mtype = 'p';
        if ((msgsnd(msgid, &my_msg, strlen(my_msg._mtxt), 0)) == -1) {
            perror("msgsnd failed");
            exit(EXIT_FAILURE);
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


/**
 * catch SIGINT and free message queue
 */
void catch_sigint(int sig_num) {
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl failed");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

