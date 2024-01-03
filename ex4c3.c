/*
 * ======================================================================
File: ex4c3.c
======================================================================
Writen by : Rotem Kashani, ID = 209073352
    Mahmood Jazmawy, ID = 211345277

 * this is the front end side of our program.
 * it prompts the user to enter a command 'q' or 'p'.
 * 'p': it waits for the user to enter a series of numbers that end with a 0.
 * it sends those number through the message queue to the server that
 * checks for prime numbers and after it gets the response it prints it 
 * to the screen.
 * 'q': it waits for the user to enter a string. it sends the string to 
 * the server that checks for palindromes and then rints the answer it
 *  got from the server to console.

 */

//====================-INCLUDES-=================
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>

//==============-DEFINE-=======================
#define MAX_LEN 1000


//=================-STRUCTS-======================
//struct to receive msg that contains a string
struct txt_msg {
    long _mtype;
    char _mtxt[MAX_LEN];
};

// struct to receive a msg with just a char
struct chr_msg {
        long _mtype;
        char _mchr;
};

//=======================-PROTOTYPES-===========================
void handle_palindrome(int msgid);

void handle_prime(int msgid);

//=====================-MAIN-====================================
int main() {
    int q_msg_id, p_msg_id;
    key_t q_key, p_key;

    // get access to the message queue
    if ((q_key = ftok(".", 'q')) == -1) {
        fprintf(stderr, "ftok() failed\n");
        exit(EXIT_FAILURE);
    }
    if ((q_msg_id = msgget(q_key, 0)) == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }

    if ((p_key = ftok(".", 'p')) == -1) {
        fprintf(stderr, "ftok() failed\n");
        exit(EXIT_FAILURE);
    }

    if ((p_msg_id = msgget(p_key, 0)) == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }


    char command;
    char final_txt[MAX_LEN];

    while (1) {
        puts("enter type and command\n");
        scanf("%c", &command);

        // reset the string to not get garbage in it
        for (int i = 0; i < MAX_LEN; ++i) {
            final_txt[i] = '\0';
        }

        if (command == 'q') { // if the user entered q
            handle_palindrome(q_msg_id);
        } else if (command == 'p') { // if the user entered p
            handle_prime(p_msg_id);
        } else { // if an invalid command was entered
            puts("invalid command!");
        }
    }
}

/**
 * this function is called when the user entered p as a command
 * it receives the numbers from the user and sends them to the prime checker server.
 */
void handle_prime(int msgid) {
    struct txt_msg my_msg, other_msg;
    char tmp[MAX_LEN];
    char final_txt[MAX_LEN];

    my_msg._mtype = 'p';

    // reset strings just in case of garbage memory
    for (int i = 0; i < MAX_LEN; ++i) {
        final_txt[i] = '\0';
        other_msg._mtxt[i] = '\0';
        tmp[i] = '\0';
    }

    // get numbers until 0 is met
    while (1) {
        scanf("%s", tmp);
        if (strcmp(tmp, "0") == 0) {
            getchar(); // get rid of whitespace
            break;
        }

        // concatenates all the numbers into one string
        strcat(final_txt, tmp);
        if (strlen(final_txt) < MAX_LEN) {
            final_txt[strlen(final_txt)] = ' ';
        }

    }

    // copy the string to our struct and send it
    strcpy(my_msg._mtxt, final_txt);
    if ((msgsnd(msgid, &my_msg, strlen(my_msg._mtxt), 0)) == -1) {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }

    // receive answer from server and print it
    if ((msgrcv(msgid, &other_msg, MAX_LEN, 'p', 0)) == -1) {
        perror("msgrcv failed");
        exit(EXIT_FAILURE);
    }
    puts(other_msg._mtxt);
}


/**
 * this function is called when the user entered q as a command
 * it receives a string from the user and sends it to the server
 * that checks for palindromes
 */
void handle_palindrome(int msgid) {
    struct txt_msg my_msg;
    struct chr_msg other_msg;
    my_msg._mtype = 'q';

    scanf("%s", my_msg._mtxt);// get the string to check
    getchar(); // get rid of whitespace


    // send the struct to the server
    if ((msgsnd(msgid, &my_msg, strlen(my_msg._mtxt), 0)) == -1) {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }

    // receive the answer from the server
    // we use another struct here to just hold a char no need for a whole string
    if ((msgrcv(msgid, &other_msg, sizeof(char), 'q', 0)) == -1) {
        perror("msgrcv failed");
        exit(EXIT_FAILURE);
    }
    printf("%c\n", other_msg._mchr);
}
