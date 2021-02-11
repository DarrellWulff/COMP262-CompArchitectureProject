#include "blackJackBehavior.h"


char *cardSuitSymbols[4] = {"♠", "♥", "♦", "♣"};

// Part of the decision making process, 'h' is for hit, 's' is for stay
char scoreTable[10][10] = {{'h','h','s','s','s','h','h','h','h','h'}, 
{'s','s','s','s','s','h','h','h','h','h'},
{'s','s','s','s','s','h','h','h','h','h'},
{'s','s','s','s','s','h','h','h','h','h'},
{'s','s','s','s','s','h','h','h','h','h'},
{'s','s','s','s','s','s','s','s','s','s'},
{'s','s','s','s','s','s','s','s','s','s'},
{'s','s','s','s','s','s','s','s','s','s'},
{'s','s','s','s','s','s','s','s','s','s'},
{'s','s','s','s','s','s','s','s','s','s'}};

CLIENT_PROFILE *gameClients[5];
int currentThreadNum;

EVENTS currentEvent;

//Initializes the gameClients and prints out the beginning text of the game.
void beginBlackJack(char *currentClients[5])
{
    for (int i = 0; i < 5; i++)
    {
        gameClients[i] = malloc(sizeof(CLIENT_PROFILE));
        gameClients[i]->name = malloc(sizeof(char)*strlen(currentClients[i]));
        strcpy(gameClients[i]->name, currentClients[i]);
        gameClients[i]->hit = false;
        gameClients[i]->bust = false;
        gameClients[i]->roundBet = 0;
        gameClients[i]->decisionScore = 0;
        gameClients[i]->winStreak = 0;
        gameClients[i]->handSize = 0;
    }
    
    printf("\n ----Welcome to the BlackJack Table----");
    printf("\n**Here are our rules** "
    "\n*The clients will place their bets"
    "\n*Then the dealer will deal cards, 2 to the clients and one to themselves"
    "\n*clients must decide to hit or stay based on the value of their hands nad the dealer's"
    "\n*The dealer then hits until they hit 17 and stay when their hand is between 18-21"
    "\n*clients win if their hands are more than the dealers, or if they have 21 or less when the dealer busts\n");
    printf("\n$$$THE PAYOUT FOR THIS CASINO IS TWO TIMES YOUR BET$$$\n");
    printf("\n Clients Josh, Carly, Sam, and Freddy gather round a smokey blackjack table...");
    printf("\n Dealer Drake %s begins to shuffle the deck as everyone else is making their bets.\n\n", gameClients[0]->name);
}

void printHand(int threadNum, char hands[][11][2])
{
    
    char *suitsEmojis[4] = {"♠", "♥", "♦", "♣"};
    int handCardsCount;
    int suitIndex[11] = {};
    
    for (int i = 0; i < 11; i++) // Looks through the hand and will print each card until a empty card slot is found
    {
        
        if (hands[threadNum][i][0] != 0) // Checks if there is a card in the i'th spot in the hand
        {
            
            switch (hands[threadNum][i][1]) // Based on the Suit value, it will set which corresponding emoji to use in the card graphic
            {
            case 'S':
                suitIndex[i] = 0;
                break;
            case 'H':
                suitIndex[i] = 1;
                break;
            case 'D':
                suitIndex[i] = 2;
                break;
            case 'C':
                suitIndex[i] = 3;
                break;
            }
        }
        else
        {
            handCardsCount = i;
            i = 11; // Prevents the for loop from continuing if there is no card in the i'th spot
        }
    }

    // Prints all cards in hand side by side
    for (int i = 0; i < handCardsCount; i++)
    {
        printf(" -------");
    }
    printf("\n");
    for (int i = 0; i < handCardsCount; i++)
    {
        printf(" |%s    |", suitsEmojis[suitIndex[i]]);
    }
    printf("\n");
    for (int i = 0; i < handCardsCount; i++)
    {
        printf(" |  %c  |", hands[threadNum][i][0]);
    }
    printf("\n");
    for (int i = 0; i < handCardsCount; i++)
    {
        printf(" |    %s|", suitsEmojis[suitIndex[i]]);
    }
    printf("\n");
    for (int i = 0; i < handCardsCount; i++)
    {
        printf(" -------");
    }
    printf("\n");
}
//Adds a large number to have a output timer, not great way to do this.
void waitForTime(double timeToWait)
{
    double timer = 0;
    timeToWait *= 10000;
    while (timer < timeToWait)
    {
        timer += 1;
    }
}

//Client Behavior Section
//gets the bets after the have been set, sends them to the correct client pointer
void getClientBets(int threadNum, int betAmount)
{
    gameClients[threadNum]->roundBet = betAmount;
}
//gets the current card points a client's hand adds up to
void getClientRoundPoints(int threadNum, int pointAmount)
{
    gameClients[threadNum]->roundPoints = pointAmount; 
}

void getClientHandSize(int threadNum, char hands[5][11][2])
{
    int curHandSize = 0;
    while (hands[threadNum][curHandSize][0] != 0)
    {
        curHandSize++;
    }
    gameClients[threadNum]->handSize = curHandSize;
}

//new round points set back to zero
void resetClientRoundPoints(int threadNum)
{
    gameClients[threadNum]->roundPoints = 0;
}

//Called when the game is starting a new round
void resetClient(int threadNum)
{
    resetClientRoundPoints(threadNum);
    gameClients[threadNum]->hit = false;
    gameClients[threadNum]->bust = false;
    gameClients[threadNum]->roundBet = 0;
    gameClients[threadNum]->decisionScore = 0;
    gameClients[threadNum]->handSize = 0;
}

// Checks values of points if its close to 17 or not
// USED BY THE DEALER
bool hitOrStay(int threadNum, char hands[][11][2])
{
    
    printf("\n\nPress ENTER to continue...\n");
    getchar();
    fflush(stdout);

    if (threadNum != 0)
    {
        printf("\n%s looks down at their hand:\n", gameClients[threadNum]->name);
    }
    printHand(threadNum, hands);
    printf("\n%s has %i points...\n", gameClients[threadNum]->name, gameClients[threadNum]->roundPoints);

    //If the dealer's hand is not == 17 or higher then they have to hit
    //otherwise they will stay.
    //Also checks if this new turn they busted
    //Was used by clients so it still checks for threadNum
    if(gameClients[threadNum]->roundPoints <= 16)
    {
        if (threadNum != 0)
        {
        printf("\n%s decides to hit!\n", gameClients[threadNum]->name);
        }
        gameClients[threadNum]->hit = true;
    }
    else if (gameClients[threadNum]->roundPoints >= 16 && gameClients[threadNum]->roundPoints <= 21)
    {
        if (threadNum != 0)
    {
        printf("\n%s decides to stay.\n", gameClients[threadNum]->name);
    }
        gameClients[threadNum]->hit = false;
    }
    else
    {
        if (threadNum == 0)
        {
            printf("\nAs dealer %s places the last card onto the table...\n", gameClients[threadNum]->name);
        }
        else if (gameClients[threadNum]->roundBet == 0)
        {
            printf("\nLooks like %s just busted! They were glad they didn't bet anything!\n", gameClients[threadNum]->name);
        }
        else
        {
            printf("\nLooks like %s just busted! They unhappily part ways with $%i.\n", gameClients[threadNum]->name, gameClients[threadNum]->roundBet);
        }
        gameClients[threadNum]->hit = false;
        gameClients[threadNum]->bust = true;
    }

    return gameClients[threadNum]->hit;
}

//simple weighted scoring AI system to pick hit or stay behavior
//looks at the hand to hit or stay based on a table that adds up
//a weight to the desicion to hit or stay. Which ever weighted
// value is higher will determine the final desicion
bool clientHitOrStay(int threadNum, char hands[][11][2])
{
    printf("\n\nPress ENTER to continue...\n");
    getchar();
    fflush(stdout);
    printf("\n%s looks down at their hand:\n", gameClients[threadNum]->name);
    printHand(threadNum, hands);
    printf("\n%s has %i points...\n", gameClients[threadNum]->name, gameClients[threadNum]->roundPoints);

    if(gameClients[threadNum]->roundPoints <= 21)
    {
        getClientHandSize(threadNum, hands);
        bool isAceFlag = false;
        int curHand = 0;
        int aceHand = 0;
        gameClients[threadNum]->hitScore = 0;
        gameClients[threadNum]->stayScore = 0;
        for (int j = 0; j < gameClients[threadNum]->handSize; j++)
        {
            //decode card value if theres an ace there two hand values to check
            int curCardVal = hands[threadNum][j][0] - '0';
            switch (curCardVal)
            {
            case 2 ... 9:
                //Already correct value, do nothing
                break;
            case 18 ... 79:
                //Value is a ten
                curCardVal = 10;
                break;
            default:
                //Value is an ace
                curCardVal = 11;
                isAceFlag = true;
                break;
            }

            curHand += curCardVal;
            if(isAceFlag)
            {
                aceHand = curHand - 10;
            }
        }
        //Send the hand to be evaluated based on the scoringTable
        clientScoreUpdate(threadNum, curHand, isAceFlag, aceHand);
        //Compare the current weights based on the hand and client's decision
        //they are leaning more towards
        if(gameClients[threadNum]->hitScore > gameClients[threadNum]->stayScore && gameClients[threadNum]->roundPoints <= 21)
        {
            gameClients[threadNum]->hit = true;
            printf("\n%s decides to hit!\n", gameClients[threadNum]->name);
        }
        else if(gameClients[threadNum]->hitScore <= gameClients[threadNum]->stayScore && gameClients[threadNum]->roundPoints <= 21)
        {
            gameClients[threadNum]->hit = false;
            if (threadNum != 0)
            {
            printf("\n%s decides to stay.\n", gameClients[threadNum]->name);
            }
        }
        else
        {
            gameClients[threadNum]->hit = false;
            gameClients[threadNum]->bust = true;
            printf("\nLooks like %s just busted! They unhappily part ways with $%i.\n", gameClients[threadNum]->name, gameClients[threadNum]->roundBet);
        }
    }
    else
    {
        gameClients[threadNum]->hit = false;
        gameClients[threadNum]->bust = true;
        printf("\nLooks like %s just busted! They unhappily part ways with $%i.\n", gameClients[threadNum]->name, gameClients[threadNum]->roundBet);
    }
    //Finally return if the current client wants to hit if they are able
    return gameClients[threadNum]->hit;
}

//Client compares and thinks according to the table
// adding to the actions hit or score based on a recommended
// action in the table.
// If the client has a winning streak they are more likely to hit
// to try and feel the rush of gambling!
void clientScoreUpdate(int threadNum, int handTotal, bool aceFlag, int aceHandTotal)
{
    if(handTotal <= 11 || (aceFlag && aceHandTotal < 11 && (aceHandTotal+10 < 17)))
    {
        gameClients[threadNum]->hitScore += 1 + gameClients[threadNum]->winStreak;
        
    }
    else
    {
        char clientChoice = scoreTable[handTotal-12][gameClients[0]->roundPoints - 2];
        switch (clientChoice)
        {
        case 'h':
            gameClients[threadNum]->hitScore += 1;
            break;
        case 's':
            gameClients[threadNum]->stayScore += 1;
            break;
        }
    }
}
  
bool checkRoundWinners(int threadNum)
{
    //Just using if statements to block off cases so that a client thread
    //is limited to how many conditionals they have to test and dialogue 
    // for each situation
    //If a client did not bust
    //Dealer didn't bust
    
    if (threadNum != 0)
    {
    waitForTime(5000);
    }
    if(gameClients[0]->bust)
    {
        //Dealer busted
        if (threadNum == 0)
        {
            printf("\nDealer %s busts! He shrugs it off.\n"
            "Everyone around the table cheers!\n", gameClients[threadNum]->name);
        }
        if(!gameClients[threadNum]->bust && threadNum != 0)
        {
            printf("\n%s feels the rush as they beat the Dealer and won $%i!\n", gameClients[threadNum]->name, gameClients[threadNum]->roundBet * 2);
            gameClients[threadNum]->winStreak++;
        }
        else if(gameClients[threadNum]->bust && threadNum != 0)
        {
            printf("\n%s ties with the Dealer so $%i is returned to %s\n"
            , gameClients[threadNum]->name, gameClients[threadNum]->roundBet, gameClients[threadNum]->name);
            gameClients[threadNum]->winStreak = 0;
        }
        
    }
    else
    {
        if (threadNum == 0)
        {
            printf("\nDealer %s does not bust and stands tall.\n", gameClients[threadNum]->name);
        }
        if(!gameClients[threadNum]->bust && threadNum != 0)
        {
            if (gameClients[threadNum]->roundPoints > gameClients[0]->roundPoints)
            {
                printf("\n%s feels the rush as they beat the Dealer and won $%i!\n", gameClients[threadNum]->name, gameClients[threadNum]->roundBet * 2);
                gameClients[threadNum]->winStreak++;
            }
            else if (gameClients[threadNum]->roundPoints == gameClients[0]->roundPoints)
            {
                printf("\n%s ties with the Dealer, nobody wins.\n$%i is returned to %s\n"
                , gameClients[threadNum]->name, gameClients[threadNum]->roundBet, gameClients[threadNum]->name);
                gameClients[threadNum]->winStreak = 0;
            }
            else
            {
                printf("\n%s loses to the Dealer.\nWith some misty eyes, %s also loses $%i.\n"
                "For Dealer %s its just another night working the Casino.\n"
                ,gameClients[threadNum]->name,gameClients[threadNum]->name, gameClients[threadNum]->roundBet, gameClients[0]->name);
                gameClients[threadNum]->winStreak = 0;
            }
        }
    }
}    
    

               
