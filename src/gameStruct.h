#ifndef HEADER_GAME_STRUCT
#define HEADER_GAME_STRUCT
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <chipmunk.h>
#include "inputManager.h"

typedef struct GameStructData GameStruct;

typedef struct SoundManagerObject {
    Music menuMusic;
    Music gameMusic;
    Sound* redPowerup;
    Sound* bluePowerup;
    Sound* slowdown;
    Sound* speedup;
    Sound* upperBouncer;
    Sound* click;
    Sound* bounce1;
    Sound* bounce2;
    Sound *flipper;
    Sound *waterSplash;
    Sound launch;
    Sound water;
    float gameMusicVolume;
} SoundManager;

typedef struct {
    int active;
    cpShape *shape;
    cpBody *body;
    GameStruct *game;
    float locationHistoryX[16];
    float locationHistoryY[16];
    int trailStartIndex;
    int type;
    int killCounter;
    int underwaterState;
} Ball;

typedef enum {
    TRANSITION_TO_MENU,
    TRANSITION_TO_GAME,
    TRANSITION_GAME_OVER
} TransitionAction;

typedef struct {
    cpShape *shape;
    cpBody *body;
    float bounceEffect;
    int type;
    int enabled;
    float angle;
    float enabledSize;
} Bumper;


typedef struct {
    float px;
    float py;
    float vx;
    float vy;
} MenuPinball;

enum CollisionTypes {
    COLLISION_WALL = 0,
	COLLISION_BALL = 1,
    COLLISION_BUMPER = 2,
    COLLISION_PADDLE = 3,
    COLLISION_LEFT_LOWER_BUMPER = 4,
    COLLISION_RIGHT_LOWER_BUMPER = 5,
    COLLISION_ONE_WAY = 6
};

typedef struct GameStructData {
    cpSpace *space;
    int numBalls;
    Ball *balls;
    int active;
    int gameState;
    long gameScore;
    long oldGameScore;
    int powerupScore;
    int powerupScoreDisplay;
    int transitionState;
    int transitionDelay;
    TransitionAction transitionTarget;
    float transitionAlpha;
    int numLives;
    int menuState;
    int nameSelectIndex;
    int nameSelectDone;
    int slowMotion;
    int slowMotionCounter;
    InputManager *input;
    float waterHeight;
    float waterHeightTarget;
    float waterHeightTimer;
    int waterPowerupState;
    int bumperPowerupState;
    int ballPowerupState;
    float redPowerupOverlay;
    float bluePowerupOverlay;
    float slowMotionFactor;
    SoundManager *sound;
    int leftFlipperState;
    int rightFlipperState;

} GameStruct;


#endif
