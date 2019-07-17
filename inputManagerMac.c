#include "raylib.h"
#include "inputManager.h"

void inputInit(){
    return;
}

void inputUpdate(){
    return;
}
void inputShutdown(int inputId){
    return;
}

int inputLeft(){
    return IsKeyDown(KEY_LEFT);
}

int inputRight(){
    return IsKeyDown(KEY_RIGHT);
}

int inputCenter(){
    return IsKeyDown(KEY_SPACE);
}

int inputLeftPressed(){
    return IsKeyPressed(KEY_LEFT);
}

int inputRightPressed(){
    return IsKeyPressed(KEY_RIGHT);
}

int inputCenterPressed(){
    return IsKeyPressed(KEY_SPACE);
}
