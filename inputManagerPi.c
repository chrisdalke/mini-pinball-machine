#include "inputManager.h"
#include <wiringSerial.h>
#include <stdlib.h>

static char tempString[64];

InputManager* inputInit(){
    InputManager *input = malloc(sizeof(InputManager));
    input->fd = serialOpen("/dev/ttyACM0",9600);
    input->keyState = 0;
    input->leftKeyPressed = 0;
    input->rightKeyPressed = 0;
    input->centerKeyPressed = 0;
    return input;
}

void inputShutdown(InputManager* input){
    serialClose(input->fd);
}

void inputUpdate(InputManager* input){
    while (serialDataAvail(input->fd) > 0){
        input->keyState = serialGetchar(input->fd);
    }
}

int inputLeft(InputManager* input){
    return (input->keyState & 4);
}

int inputRight(InputManager* input){
    return (input->keyState & 2);
}

int inputCenter(InputManager* input){
    return (input->keyState & 1);
}

int inputLeftPressed(InputManager* input){
    if (inputLeft(input)){
        if (input->leftKeyPressed == 0){
            input->leftKeyPressed = 1;
            return 1;
        }
    } else {
        input->leftKeyPressed = 0;
    }
    return 0;
}

int inputRightPressed(InputManager* input){
    if (inputRight(input)){
        if (input->rightKeyPressed == 0){
            input->rightKeyPressed = 1;
            return 1;
        }
    } else {
        input->rightKeyPressed = 0;
    }
    return 0;
}

int inputCenterPressed(InputManager* input){
    if (inputCenter(input)){
        if (input->centerKeyPressed == 0){
            input->centerKeyPressed = 1;
            return 1;
        }
    } else {
        input->centerKeyPressed = 0;
    }
    return 0;
}

// Send game state
void inputSetGameState(InputManager* input, InputGameState state){
    switch (state){
        case STATE_MENU: {
            sprintf(tempString,"m\n");
            serialPuts(input->fd,tempString);
            serialFlush(input->fd);
            break;
        }
        case STATE_GAME: {
            sprintf(tempString,"r\n");
            serialPuts(input->fd,tempString);
            serialFlush(input->fd);
            break;
        }
        case STATE_GAME_OVER: {
            sprintf(tempString,"g\n");
            serialPuts(input->fd,tempString);
            serialFlush(input->fd);
            break;
        }
    }
}
void inputSetScore(InputManager *input, long score){
    sprintf(tempString,"s=%ld\n",score);
    serialPuts(input->fd,tempString);
    serialFlush(input->fd);
}
void inputSetNumBalls(InputManager *input, int numBalls){
    sprintf(tempString,"b=%d\n",numBalls);
    serialPuts(input->fd,tempString);
    serialFlush(input->fd);
}
