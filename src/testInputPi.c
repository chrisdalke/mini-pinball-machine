// Tests the pi input system
// Use the following command to compile:
// gcc -o test_input testInputPi.c -O1 -s -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -std=gnu99 -I.-I/opt/vc/include -L/opt/vc/lib

#include "inputManager.h"

int fd = inputInit();

while (true){
    inputUpdate(fd);
}
