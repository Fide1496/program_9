#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

int guess[2] = {0, 0};
int cmp[2] = {0, 0};
int rdy[4] = {0, 0, 0, 0};
int wins[2] = {0, 0};

pthread_mutex_t mtx[3];
pthread_cond_t cnd[3];

// Check error function
int checkError(int val, const char *msg) {
    if (val == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return val;
}

// Check thread error function
int checkTError(int val, const char *msg) {
    if (val == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

void *player_one(void * args){

    int min = 0;
    int max = 100;

    while(1){

        int min = 0;
        int max = 100;

        pthread_mutex_lock(&mtx[2]);

        while (rdy[2] == 0) pthread_cond_wait(&cnd[2], &mtx[2]);

        pthread_mutex_unlock(&mtx[2]);
        rdy[2]=0;

        while(1){
            guess[0] = (min+max) /2;
            printf("Player 1 guess: %d\n", guess[0]);

            pthread_mutex_lock(&mtx[0]);
            
            pthread_cond_signal(&cnd[0]);
            while (rdy[2] == 0) {
                pthread_cond_wait(&cnd[0], &mtx[0]);
            }
            pthread_mutex_unlock(&mtx[0]);

            if (cmp[0] < 0)
                min = guess[0];
            else if (cmp[0] > 0)
                max = guess[0];
            else
                break;
        }
    }
    pthread_exit(NULL);
}
void *player_two(void * args) {

    srand(time(NULL));

    int min = 0;
    int max = 101;

    while (1) {

        int min = 0;
        int max = 101;

        pthread_mutex_lock(&mtx[2]);

        while (rdy[3] == 0) pthread_cond_wait(&cnd[2], &mtx[2]);        
        
        pthread_mutex_unlock(&mtx[2]);
        rdy[3] = 0;

        while (1) {
            guess[1] = min + rand() % (max - min + 1);
            printf("Player 2 guess: %d\n", guess[1]);

            pthread_mutex_lock(&mtx[1]);
            pthread_cond_signal(&cnd[1]);

            while (rdy[3] == 0) {
                pthread_cond_wait(&cnd[1], &mtx[1]);
            }
            pthread_mutex_unlock(&mtx[1]);

            if (cmp[1] < 0)
                min = guess[1];
            else if (cmp[1] > 0)
                max = guess[1];
            else
                break;

        }
    }
    pthread_exit(NULL);
}

void *referee(void * arg) {

    int game_num =10;

    for (game_num = 1; game_num <= 10; game_num++) {

        pthread_mutex_lock(&mtx[2]);

        int target = 1 + rand() % 100;

        rdy[2] = 1;
        rdy[3] = 1;

        pthread_cond_broadcast(&cnd[2]);
        
        pthread_mutex_unlock(&mtx[2]);

        printf("Game %d:\n", game_num);
        

        while (1) {
            sleep(1);

            pthread_mutex_lock(&mtx[0]);
            pthread_mutex_lock(&mtx[1]);

            
            rdy[0] = 0;
            rdy[1] = 0;

            cmp[0] = guess[0]-target;
            cmp[1] = guess[1] -target;

            rdy[0] = 1;
            rdy[1] = 1;

            pthread_cond_signal(&cnd[0]);
            pthread_cond_signal(&cnd[1]);

            pthread_mutex_unlock(&mtx[0]);
            pthread_mutex_unlock(&mtx[1]);


            if (cmp[0] == 0) {
                printf("Player 1 guessed the correct number: %d\n", target);
                cmp[1]=0;
                wins[0]++;
                break;
            }
            if (cmp[1] == 0) {
                printf("Player 2 guessed the correct number: %d\n", target);
                cmp[0]=0;
                wins[1]++;
                break;
            }
        }

        printf("Player 1 Wins: %d\nPlayer 2 Wins: %d\n", wins[0], wins[1]);
    }
    pthread_cond_broadcast(&cnd[0]);
    pthread_cond_broadcast(&cnd[1]);
    pthread_cond_broadcast(&cnd[2]);

    printf("Player 1 Wins: %d\nPlayer 2 Wins: %d\n", wins[0], wins[1]);

    pthread_exit(NULL);
}


int main(int argc, char *argv[]){

    srand(time(NULL));
    pthread_t thrds[3];
    void *(*fcns[3])(void *) = {player_one, player_two, referee};
    int i, j;

    rdy[0] = 0;
    rdy[1] = 0;

    for (i = 0; i < 3; i++) {
        pthread_create(&thrds[i], NULL,fcns[i], NULL);
    }

    for (j = 0; j < 3; j++) {
        pthread_join(thrds[j], NULL);
    }

    exit(EXIT_SUCCESS);
}