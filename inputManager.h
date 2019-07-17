#ifndef HEADER_INPUT
#define HEADER_INPUT

int inputInit();
void inputShutdown(int inputId);
void inputUpdate(int inputId);
int inputLeft(int inputId);
int inputRight(int inputId);
int inputCenter(int inputId);
int inputLeftPressed(int inputId);
int inputRightPressed(int inputId);
int inputCenterPressed(int inputId);

#endif
