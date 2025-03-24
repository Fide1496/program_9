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

void *player_one(void *arg){
    int min = 0;
    int max = 0;

    while(1){
        pthread_mutex_lock(&mtx[2]);
        while(rdy[2]==0){
            pthread_cond_wait(&cnd[2],&mtx[2]);
        }
        pthread_mutex_unlock(&mtx[2]);
        rdy[2]=0;
        while(1){
            guess[0] = (min+max) /2;

            pthread_mutex_lock(&mtx[0]);
            rdy[0] = 1;
            pthread_cond_signal(&cnd[0]);
            while (rdy[0] == 1) {
                pthread_cond_wait(&cnd[0], &mtx[0]);
            }
            pthread_mutex_unlock(&mtx[0]);

            if (cmp[0] < 0)
                min = guess[0] + 1;
            else if (cmp[0] > 0)
                max = guess[0] - 1;
            else
                break;
        }
    }

    return NULL;

}
void *player_two(void *arg) {
    int min = 0, max = 100;
    srand(time(NULL));

    while (1) {
        pthread_mutex_lock(&mtx[2]);
        while (rdy[3] == 0) {
            pthread_cond_wait(&cnd[2], &mtx[2]);
        }
        pthread_mutex_unlock(&mtx[2]);
        rdy[3] = 0;

        while (1) {
            guess[1] = min + rand() % (max - min + 1);

            pthread_mutex_lock(&mtx[1]);
            rdy[1] = 1;
            pthread_cond_signal(&cnd[1]);
            while (rdy[1] == 1) {
                pthread_cond_wait(&cnd[1], &mtx[1]);
            }
            pthread_mutex_unlock(&mtx[1]);

            if (cmp[1] < 0)
                min = guess[1] + 1;
            else if (cmp[1] > 0)
                max = guess[1] - 1;
            else
                break;
        }
    }
    return NULL;
}

void *referee(void *arg) {
    for (int game = 1; game <= 10; game++) {
        int target = 1 + rand() % 100;

        pthread_mutex_lock(&mtx[2]);
        rdy[2] = 1;
        rdy[3] = 1;
        pthread_cond_broadcast(&cnd[2]);
        pthread_mutex_unlock(&mtx[2]);

        printf("Game %d:\n", game);

        while (1) {
            sleep(1);

            pthread_mutex_lock(&mtx[0]);
            pthread_mutex_lock(&mtx[1]);

            rdy[0] = 0;
            rdy[1] = 0;

            cmp[0] = guess[0] - target;
            cmp[1] = guess[1] - target;

            if (cmp[0] == 0 || cmp[1] == 0) {
                cmp[0] = cmp[1] = 0;
                printf("Game %d: A player guessed the correct number!\n", game);
                pthread_mutex_unlock(&mtx[1]);
                pthread_mutex_unlock(&mtx[0]);
                break;
            }

            rdy[0] = 1;
            rdy[1] = 1;
            pthread_cond_signal(&cnd[0]);
            pthread_cond_signal(&cnd[1]);

            pthread_mutex_unlock(&mtx[1]);
            pthread_mutex_unlock(&mtx[0]);
        }
    }
    return NULL;
}


int main(){

    srand(time(NULL));

    for (int i =0; i < 3; i++){

        pthread_mutex_init(&mtx[i], NULL);
        pthread_mutex_init(&mtx[i], NULL);
    }

    pthread_t p1,p2, ref;

    pthread_create(&p1, NULL,player_one, NULL);
    pthread_create(&p2,NULL,player_two,NULL);
    pthread_create(&ref,NULL,referee,NULL);

    pthread_join(ref,NULL);

    exit(EXIT_SUCCESS);
}