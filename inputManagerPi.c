#include "inputManager.h"
#include <wiringSerial.h>
#include <stdlib.h>

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
    return (input->keyState & 1);
}

int inputCenter(InputManager* input){
    return (input->keyState & 2);
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
