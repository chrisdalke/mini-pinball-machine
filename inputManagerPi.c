#include "inputManager.h"
#include <wiringSerial.h>

int inputInit(){
    int fd = serialOpen("/dev/ttyACM0",9600);
    return 0;
}

void inputShutdown(int inputId){
    serialClose(inputId);
}

void inputUpdate(int inputId){
    if (serialDataAvail(input) > 0){
        int nextChar = serialGetchar(inputId);
        printf("%d\n",nextChar);
    }
    return;
}

int inputLeft(int inputId){
    return 0;
}

int inputRight(int inputId){
    return 0;
}

int inputCenter(int inputId){
    return 0;
}

int inputLeftPressed(int inputId){
    return 0;
}

int inputRightPressed(int inputId){
    return 0;
}

int inputCenterPressed(int inputId){
    return 0;
}
