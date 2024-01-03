/*
 * ======================================================================
File: ex4c2.c
======================================================================
Writen by : Rotem Kashani, ID = 209073352
    Mahmood Jazmawy, ID = 211345277

 * This program is our server that checks for palindromes numbers.
 * it receives a string from the client through the message queue.
 * it checks if the string is a palindrome it sends '+' back through the queue.
 * if it's not a palindrome it sends '-' back
 
 */

//====================-INCLUDES-=================
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

//==============-DEFINE-=======================
#define MAX_LEN 1000


//=================-STRUCTS-======================
struct txt_msg {
    long _mtype;
    char _mtxt[MAX_LEN];
};

struct chr_msg {
    long _mtype;
    char _mchr;
};

//=======================-PROTOTYPES-===========================
void catch_sigint(int sig_num);

bool is_palindrome(char str[]);

//=======-GLOBALS-==================
int msgid;


//===========-MAIN-==================
int main() {
    struct txt_msg my_msg;
    struct chr_msg ret_msg;
    ret_msg._mtype = 'q';
    my_msg._mtype = 'q';

    key_t key;
    signal(SIGINT, catch_sigint);


    // create message pipe
    if ((key = ftok(".", 'q')) == -1) {
        fprintf(stderr, "ftok() failed\n");
        exit(EXIT_FAILURE);
    }
    if ((msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0600)) == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }


    while (1) {
        // receive string to check for palindrome
        if ((msgrcv(msgid, &my_msg, MAX_LEN, 'q', 0)) == -1) {
            perror("msgrcv failed");
            exit(EXIT_FAILURE);
        }

        //check if palindrome
        if (is_palindrome(my_msg._mtxt)) {
            ret_msg._mchr = '+';
        } else {
            ret_msg._mchr = '-';
        }

        // send answer to the client
        if ((msgsnd(msgid, &ret_msg, sizeof(char), 0)) == -1) {
            perror("msgsnd failed");
            exit(EXIT_FAILURE);
        }
    }
}


/**
 * This function checks if a string is a palindrome or not
 */
bool is_palindrome(char str[]){
    size_t l = 0;
    size_t h = strlen(str) - 1;

    // Keep comparing characters while they are same
    while (h > l) {
        if (str[l++] != str[h--]) {
            return false; // is not palindrome
        }
    }
    return true; // is palindrome
}


/**
 * catch SIGINT, frees queue when signal is raised
 */
void catch_sigint(int sig_num) {
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl failed");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

