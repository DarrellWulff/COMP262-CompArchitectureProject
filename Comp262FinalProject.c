/******************************************************************************
* COMP 262 Final Project: Casino Royale: Parallelized Blackjack Edition
* Authors: Taseen Hafiz, Darrell Wulff
* Date: 11/15/19
* To run: gcc -fopenmp -o final.out Comp262FinalProject.c -lm blackJackBehavior.c
* For Lab Macs: clang -Xpreprocessor -fopenmp -lomp -o final.out Comp262FinalProject.c blackJackBehavior.c
* NOTE: Throughout the program, "omp_get_thread_num()" is going to be used to determine which client/dealer the action applies to.
*
*Summary of the implementation:

    This implementation of BlackJack uses the power of OpenMP's 
    parallel for loops and thread numbering. After intitalizing
    the variables needed for a game of blackjack we enter into
    the main game loop. This while loop will continue as long
    as the user keeps the game state set to true. Leveraging 
    the parallel for loop we staticly schedule 5 threads to
    be the participants of the game. Until the threads hit 
    the ordered section of the loop, it does not matter the
    order in which initial bets, hands, points, and other data
    is calculated. This allows for the game to take advantage
    of parallel calculations. Once the code reaches the ordered
    area the threads will run one after the other to decide to hit
    or stay. Thread 0 acts as the dealer and the others as clients,
    so there is some careful consideration as to when the dealer
    gets to act ever when its not the thread running. The clients
    also decide to hit and stay based on a scoring AI system. The
    AI scoring system is not in parallel but makes the game more
    interesting as their is more thought put into their actions. Then
    the winners are found in a parallel loop since the order again
    is not critical. A secondfile was created to abstract away some 
    functionality and read in data based on the thread number. 
    Overall this implementation is based around the simple idea of 
    threads being it's own entity in a game.
*-------------------------------------
*
******************************************************************************/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include "blackJackBehavior.h"


void addUpPoints(char hands[][11][2], int points[], int threadNum)
{
    int aCount = 0;                   // This will count how many As are in the hand so that it is known how many points need to be deducted later, if any
    points[threadNum] = 0; // Resets the total points so that whenever addUpPoints() is called, the proper amount of points is always calculated

    for (int i = 0; i < 11; i++)
    {
        if (hands[threadNum][i][0] != 0) // Checks if there is a card in the i'th spot in the hand
        {
            switch (hands[threadNum][i][0]) // Based on the card value, the appropriate amount of points are added. '2-9' represet themselves in points. 'T', 'J', 'Q', and 'K' represent 10 points. 'A' represents 11 points unless doing so would make the total points greater than 21, in which case 'A' will be 1 point.
            {
            case 'A':
                points[threadNum] += 11; // Adds 11 points to clients' or dealer's point total. Later it will be seen if counting A as 11 points will make it greater than 21, in which case, 10 points will be deducted so that the A is only counted as a 1
                aCount++;
                break;
            case '2':
                points[threadNum] += 2;
                break;
            case '3':
                points[threadNum] += 3;
                break;
            case '4':
                points[threadNum] += 4;
                break;
            case '5':
                points[threadNum] += 5;
                break;
            case '6':
                points[threadNum] += 6;
                break;
            case '7':
                points[threadNum] += 7;
                break;
            case '8':
                points[threadNum] += 8;
                break;
            case '9':
                points[threadNum] += 9;
                break;
            case 'T':
                points[threadNum] += 10;
                break;
            case 'J':
                points[threadNum] += 10;
                break;
            case 'Q':
                points[threadNum] += 10;
                break;
            case 'K':
                points[threadNum] += 10;
                break;
            }
        }
    }

    // If there are As in the hand, then it will check if the total points are greater than 21. If so, then 10 points will be deducted
    for (int i = 0; i < aCount; i++)
    {
        if (points[omp_get_thread_num()] > 21)
        {
            points[omp_get_thread_num()] -= 10;
        }
    }

}

void drawCard(char deck[][3], char hands[][11][2], int threadNum)
{
    bool continueLoop; // The loop will not be triggered if the first random card has not been drawn before
    int randCardIndex;

    // Continues to pick cards until it finds one that has not already been drawn
    do
    {
        randCardIndex = (rand() % 53);   // Picks random card from the deck to draw
        if (deck[randCardIndex][2] == 0) // Checks if picked card has already been used
        {
            continueLoop = false; // Since an available card was found, the loop is broken
        }
        else if (deck[randCardIndex][2] == 1)
        {
            continueLoop = true;
        }
    } while (continueLoop);

    // Finds the next empty space in the client's/dealer's hand, places that card in the hand, and marks the card in the deck that it has been used
    for (int i = 0; i < 11; i++)
    {
        if (hands[threadNum][i][0] == 0) // Checks if there is a card in the i'th spot in the hand
        {
            hands[threadNum][i][0] = deck[randCardIndex][0]; // Records the card value in hand
            hands[threadNum][i][1] = deck[randCardIndex][1]; // Records the card suit in hand
            deck[randCardIndex][2] = 1;                                 // Marks the card in deck as not available
            i = 11;                                                     // Prevents the loop from continuing once an available card is found
        }
    }
}

void dealCards(char deck[][3], char hands[][11][2])
{
    // Draws 2 cards into the hand of the client/dealer (NOTE: Dealer will only take one card at this point)
    if (omp_get_thread_num() == 0) // Checks if the one being dealt is a client or the dealer
    {
        drawCard(deck, hands, 0); // Deals only one card since they are the dealer
    }
    else
    {
        drawCard(deck, hands, omp_get_thread_num()); // Deals two card since they are
        drawCard(deck, hands, omp_get_thread_num());
    }
}

void setBets(int balance[], int bets[])
{
    int betTemp; // Temporarily holds randomly generated bets

    // Sets bets for everyone but the dealer and deducts that amount from the balance
    betTemp = (rand() % 1001);                // Selects a random amount to bet from $0-1000
    balance[omp_get_thread_num()] -= betTemp; // Deducts from balance
    bets[omp_get_thread_num()] = betTemp;     // Stores the bet
}
//reset data in main for next round
void resetClientData(int threadNum, char hands[5][11][2], int balance[5], int points[5])
{
    balance[threadNum] = 0;
    points[threadNum] = 0;
    //hands has to be reset with two loops
    int curCardVal = 0;
    while (curCardVal < 11)
    {
        int curCardSuit = 0;
        while (curCardSuit < 2)
        {
            hands[threadNum][curCardVal][curCardSuit] = 0;
            curCardSuit++;
        }
        curCardVal++;
    }
}

int main()
{
    omp_set_num_threads(5); // One thread per client and an additional one for the dealer
    double start;
    double end;
    start = omp_get_wtime();

    bool gamestate = true;
    
    char *clients[5] = {"Drake", "Josh", "Carly", "Sam", "Freddy"}; // The clients including the dealer (Drake is appointed dealer)
    char hands[5][11][2] = {};                                      // Initializes hand, first dimension is who's hand it is, second dimension is the value of the hand and third dimension is the suit of the hand. Max amount of cards in a hand is 11 until it reaches 21.
    int balance[5] = {0, 1000, 1000, 1000, 1000};                   // Initializes everyone's money in dollars (element 0 is set to 0 as dealer does not bet any money)
    int bets[5] = {};                                               // Holds what everyone is betting. Although the dealer (element 0) is not betting anything, array length is kept at 5 for consistency
    int points[5] = {};                                             // Initializes everyone's points. This will keep track of how many points everyone's hands add up to
    
    char suits[4] = {'S', 'H', 'D', 'C'};                                                   // The suits of the cards where S is for Spades, H is for Hearts, D is for Diamonds, and C is for Clubs
    char cardValue[13] = {'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K'}; // The values of the cards where ints represents their own values. T, J, Q, & K all represent the value 10 and A is either 1 or 11 depending on which is best
    char deck[52][3] = {};                                                                  // Initializes deck of cards, first dimension is the value, second dimension is the suit and if the card has been used or not (0 for available, 1 for used)

    // Creates deck
    int cardValueIndex = 0;
    int suitIndex = 0;

    for (int i = 0; i < 52; i++)
    {
        deck[i][0] = cardValue[cardValueIndex % 13]; // Sets value of the card
        deck[i][1] = suits[suitIndex % 4];           // Sets suit of the card
        cardValueIndex++;
        suitIndex++;
    }

    //Set Up behaviors and dialogue
    beginBlackJack(clients);

    //MAIN GAME LOOP
    while (gamestate) // Keeps the games going until input is taken to end it
    {
        #pragma omp parallel for ordered schedule(static,1)// Parallelizes for loop, giving each thread 1 loop to run
        for (int i = 0; i < 5; i++)           // Splits each client into 5 threads, one for each client and the dealer. Dealer is thread 0, while clients are threads 1-3
        {
            // Sets a seed for rand() in each thread based on time and which thread it's in
            srand((int)(time(NULL) + pow((omp_get_thread_num() + 1), 2)) % __INT_MAX__); 
            // Skips setting bet for dealer since they do not bet money
            if (i != 0)
            {
                setBets(balance, bets);
                //Grabs the bet and places it into the client struct to be used later
                getClientBets(omp_get_thread_num(), bets[omp_get_thread_num()]);
            }
        
            dealCards(deck, hands);     // The dealer is dealt one card while each client is dealt two
            addUpPoints(hands, points, omp_get_thread_num()); // Adds up the points in order to decide if client wants to hit or stay
            //Grabs the added up points to be used in deciding whether to hit or stay
            getClientRoundPoints(omp_get_thread_num(), points[omp_get_thread_num()]);
            
            //Serial portion
            #pragma omp ordered
            {
                
                if (omp_get_thread_num() != 0) // This is the clients deciding whether to hit or stay NOTE: Clients do not play perfectly but make mistakes to be entertaining
                {
                    //Have the threads go in order to add to hands one at a time
                    //Dealer reveals their first card to the clients
                    while (clientHitOrStay(omp_get_thread_num(),hands))
                    {
                        drawCard(deck, hands, omp_get_thread_num());
                        addUpPoints(hands, points, omp_get_thread_num());
                        getClientRoundPoints(omp_get_thread_num(), points[omp_get_thread_num()]);
                    }
                    //Check who wins money and the dealer's turn when the last thread is running
                    if(omp_get_thread_num() == 4)
                    {
                        printf("\n***DEALER TURN***\n");
                        printf("The converstion at the BlackJack Table stops and the tension in the air thickens as dealer Drake reveals the rest of the cards... \n");
                        while (hitOrStay(0,hands))
                        {
                            drawCard(deck, hands, 0);
                            addUpPoints(hands, points, 0);
                            getClientRoundPoints(0, points[0]);
                        }
                    }
                }
                else
                {
                    //Dealer Thread, print all calculated bets
                    //then reveal the first card to the clients
                    for (int j = 1; j < 5; j++)
                    {
                        printf("%s: I bet $%d!\n", clients[j], bets[j]);
                    }
                    printf("\nDealer Drake reveals the first card!\n");
                    fflush(stdout);
                    printHand(0, hands);
                }
                
            } 
        }

        //Check winners of round, order does not matter.
        #pragma omp parallel for schedule(auto)
        for (int i = 0; i < 5; i++)
        {
            checkRoundWinners(i);
        }

        //Makes sure that getchar() does not continue by reading in a \n char
        //since \n will continue the loop without user input
        char continueGame;
        printf("\nContinue with another game? (Y/N): ");
        continueGame = getchar();
        while (continueGame == '\n')
        {
            continueGame = getchar();
        }

        if ((continueGame == 'N') || (continueGame == 'n')) // Continues to another game of Blackjack if Y is inputted and will end the while loop if N is entered
        {
            gamestate = false;
        }
        else
        {
            //Reset the game deck
            cardValueIndex = 0;
            suitIndex = 0;
            for (int i = 0; i < 52; i++)
            {
                deck[i][0] = cardValue[cardValueIndex % 13]; 
                deck[i][1] = suits[suitIndex % 4];           
                cardValueIndex++;
                suitIndex++;
            }
            for (int j = 0; j < 5; j++)
            {
                //Resets most the game data here
                //Runs through and initializes all to 0
                resetClientData(j, hands, balance, points);
                resetClient(j);
            }
        }
    }

    return 0;
}