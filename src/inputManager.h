#ifndef HEADER_INPUT
#define HEADER_INPUT

typedef struct {
    int fd;
    int keyState;
    int leftKeyPressed;
    int rightKeyPressed;
    int centerKeyPressed;
} InputManager;

typedef enum {
    STATE_MENU,
    STATE_GAME,
    STATE_GAME_OVER
} InputGameState;

InputManager* inputInit();
void inputShutdown(InputManager* input);
void inputUpdate(InputManager* input);
int inputLeft(InputManager* input);
int inputRight(InputManager* input);
int inputCenter(InputManager* input);
int inputLeftPressed(InputManager* input);
int inputRightPressed(InputManager* input);
int inputCenterPressed(InputManager* input);
void inputSetGameState(InputManager* input, InputGameState state);
void inputSetScore(InputManager *input, long score);
void inputSetNumBalls(InputManager *input, int numBalls);

#endif
