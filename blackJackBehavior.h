#ifndef BLACKJACKBEHAVIOR_H
#define BLACKJACKBEHAVIOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>

typedef struct client_profile
{
    char *name;
    int thread_num;
    int roundPoints;
    int roundBet;
    int handSize;
    int decisionScore;
    int hitScore;
    int stayScore;
    int winStreak;
    bool hit;
    bool bust;
    bool win;
}CLIENT_PROFILE;

typedef enum
{
    OWNER_EVENT = 0,
    METEOR_EVENT,
    FIRE_EVENT,
    JAMESBOND_EVENT
}EVENTS;

void beginBlackJack(char *currentClients[5]);

void printHand(int userThread, char hands[][11][2]);
void waitForTime(double timeToWait);

//Get data from main game
void getClientBets(int threadNum, int betAmount);
void getClientRoundPoints(int threadNum, int pointAmount);
void getClientHandSize(int threadNum, char hands[5][11][2]);
void resetClientRoundPoints(int threadNum);
void resetClient(int threadNum);

bool hitOrStay(int threadNum, char hands[][11][2]);
bool clientHitOrStay(int threadNum, char hands[][11][2]);
void clientScoreUpdate(int threadNum, int handTotal, bool aceFlag, int aceHandTotal);

bool checkRoundWinners();

#endif