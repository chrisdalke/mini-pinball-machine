#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <chipmunk.h>
#include "constants.h"
#include "physicsDebugDraw.h"
#include "inputManager.h"
#include "scores.h"
#include "soundManager.h"
#include "gameStruct.h"

#define DEG_TO_RAD (3.14159265 / 180.0)
#define RAD_TO_DEG (180.0 / 3.14159265)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#if defined(PLATFORM_RPI)
    #define GLSL_VERSION            100
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            330
#endif

long long millis() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

const int numWalls = 128;
const int maxBalls = 256;
const float ballSize = 5;

static float slowMotionFactor = 1.0f;
static float iceOverlayAlpha = 0.0f;
// Collision handlers

static float leftLowerBumperAnim = 0.0f;
static float rightLowerBumperAnim = 0.0f;

static float multiballOverlayY = 0.0f;

static cpBool CollisionHandlerLeftLowerBumper(cpArbiter *arb, cpSpace *space, void *ignore){
	CP_ARBITER_GET_SHAPES(arb, a, b);
    Ball *ball = (Ball *)cpShapeGetUserData(a);
    leftLowerBumperAnim = 1.0f;
    (ball->game)->gameScore += 25;
    if ((ball->game)->waterPowerupState == 0){
        (ball->game)->powerupScore += 25;
    }
    playBounce2((ball->game)->sound);
    return cpTrue;
}
static cpBool CollisionHandlerRightLowerBumper(cpArbiter *arb, cpSpace *space, void *ignore){
	CP_ARBITER_GET_SHAPES(arb, a, b);
    Ball *ball = (Ball *)cpShapeGetUserData(a);
    rightLowerBumperAnim = 1.0f;
    (ball->game)->gameScore += 25;
    if ((ball->game)->waterPowerupState == 0){
        (ball->game)->powerupScore += 25;
    }
    playBounce2((ball->game)->sound);
    return cpTrue;
}
static cpBool CollisionHandlerBallFlipper(cpArbiter *arb, cpSpace *space, void *ignore){
	CP_ARBITER_GET_SHAPES(arb, a, b);
    Ball *ball = (Ball *)cpShapeGetUserData(a);
    ball->killCounter = 0;
    return cpTrue;
}

static cpBool CollisionHandlerBallBumper(cpArbiter *arb, cpSpace *space, void *ignore){
	CP_ARBITER_GET_SHAPES(arb, a, b);
	Bumper* bumper = (Bumper *)cpShapeGetUserData(b);
    Ball *ball = (Ball *)cpShapeGetUserData(a);

    if (bumper->type == 0){
        // On the bumper object, set the collision effect
        bumper->bounceEffect = 10.0f;
        //if (ball->type == 0){
        (ball->game)->gameScore += 50;
        if ((ball->game)->waterPowerupState == 0){
            (ball->game)->powerupScore += 50;
        }
        playUpperBouncerSound((ball->game)->sound);
        //}

        // On the ball object, add a random velocity
        //cpBodyApplyImpulseAtLocalPoint(ball->body,cpv(rand() % 20 - 10, rand() % 20 - 10),cpvzero);
    	return cpTrue;
    } else if (bumper->type == 1){
        (ball->game)->slowMotion = 1;
        (ball->game)->slowMotionCounter = 1200;
        (ball->game)->gameScore += 1000;
        if ((ball->game)->waterPowerupState == 0){
            (ball->game)->powerupScore += 1000;
        }
        playSlowdownSound((ball->game)->sound);
        bumper->bounceEffect = 20.0f;
    } else if (bumper->type == 2){
        if (bumper->enabled == 1){
            (ball->game)->gameScore += 50;
            if ((ball->game)->waterPowerupState == 0){
                (ball->game)->powerupScore += 50;
            }
            bumper->enabled = 0;
            playBounce((ball->game)->sound);
        }
    } else if (bumper->type == 3){
        if (bumper->enabled == 1){
            (ball->game)->gameScore += 50;
            if ((ball->game)->waterPowerupState == 0){
                (ball->game)->powerupScore += 50;
            }
            bumper->enabled = 0;
            playBounce((ball->game)->sound);
        }
    } else if (bumper->type == 4){
        if (bumper->enabled == 1){
            bumper->bounceEffect = 10.0f;
            (ball->game)->gameScore += 250;
            if ((ball->game)->waterPowerupState == 0){
                (ball->game)->powerupScore += 250;
            }
            bumper->enabled = 0;
            playBounce((ball->game)->sound);
            return cpTrue;
        } else {
            return cpFalse;
        }
    } else {
        (ball->game)->gameScore += 25;
        if ((ball->game)->waterPowerupState == 0){
            (ball->game)->powerupScore += 25;
        }
        bumper->enabled = 0;
    }

	return cpFalse;
}

static cpBool CollisionOneWay(cpArbiter *arb, cpSpace *space, void *ignore){
	CP_ARBITER_GET_SHAPES(arb, a, b);
    printf("%f\n",cpvdot(cpArbiterGetNormal(arb), cpv(0,1)));
	if(cpvdot(cpArbiterGetNormal(arb), cpv(0,1)) < 0){
		return cpArbiterIgnore(arb);
	}

	return cpTrue;
}

// Add ball function
void addBall(GameStruct *game, float px, float py, float vx, float vy,int type){
    if (game->numBalls < maxBalls){
        game->numBalls++;
        // Find the first index that isn't active
        int ballIndex = 0;
        for (int i = 0; i < maxBalls; i++){
            if (game->balls[i].active == 0){
                ballIndex = i;
                break;
            }
        }

        float radius = ballSize / 2.0;
        float mass = 1.0;
        if (type == 2){
            radius = 10.0f;
            mass = 2.0f;
        }
        cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
        game->balls[ballIndex].body = cpSpaceAddBody(game->space,cpBodyNew(mass,moment));
        cpBodySetPosition(game->balls[ballIndex].body,cpv(px,py));
        cpBodySetVelocity(game->balls[ballIndex].body,cpv(vx,vy));
        game->balls[ballIndex].shape = cpSpaceAddShape(game->space,cpCircleShapeNew(game->balls[ballIndex].body,radius,cpvzero));
        cpShapeSetFriction(game->balls[ballIndex].shape,0.0);
        cpShapeSetElasticity(game->balls[ballIndex].shape,0.7);
        cpShapeSetCollisionType(game->balls[ballIndex].shape, COLLISION_BALL);
        cpShapeSetUserData(game->balls[ballIndex].shape,&(game->balls[ballIndex]));
        game->balls[ballIndex].active = 1;
        game->balls[ballIndex].game = game;
        game->balls[ballIndex].trailStartIndex = 0;
        game->balls[ballIndex].type = type;
        game->balls[ballIndex].killCounter = 0;
        if (type == 0){
            game->slowMotion = 0;
        }

        for (int i = 0; i< 16; i++){
            game->balls[ballIndex].locationHistoryX[i] = px;
            game->balls[ballIndex].locationHistoryY[i] = py;
        }

        playLaunch(game->sound);
    }
}

// Write circle wall segments in a clockwise direction starting at degStart and ending at degEnd
void writeCircleWallSegment(float walls[numWalls][4],int segmentIndex,int numSegments,float degStart,float degEnd,float centerX, float centerY, float radius){
    float totalDegreeRange = fabsf(degEnd - degStart);
    float degPerPoint = totalDegreeRange / (numSegments + 1);

    degStart -= 90;

    // Iterate through numSegments + 1 points.
    float prevPointX = centerX + (cos(degStart * DEG_TO_RAD) * radius);
    float prevPointY = centerY + (sin(degStart * DEG_TO_RAD) * radius);
    float curPointX = 0;
    float curPointY = 0;
    for (int i = 1; i <= numSegments; i++){
        curPointX = centerX + (cos((degStart + (i * degPerPoint)) * DEG_TO_RAD) * radius);
        curPointY = centerY + (sin((degStart + (i * degPerPoint)) * DEG_TO_RAD) * radius);
        walls[segmentIndex + i - 1][0] = prevPointX;
        walls[segmentIndex + i - 1][1] = prevPointY;
        walls[segmentIndex + i - 1][2] = curPointX;
        walls[segmentIndex + i - 1][3] = curPointY;
        prevPointX = curPointX;
        prevPointY = curPointY;
    }
}



// Debug drawing functions
static void
ChipmunkDebugDrawCirclePointer(cpVect p, cpFloat a, cpFloat r, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data)
{ChipmunkDebugDrawCircle(p, a, r, outline, fill);}

static void
ChipmunkDebugDrawSegmentPointer(cpVect a, cpVect b, cpSpaceDebugColor color, cpDataPointer data)
{ChipmunkDebugDrawSegment(a, b, color);}

static void
ChipmunkDebugDrawFatSegmentPointer(cpVect a, cpVect b, cpFloat r, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data)
{ChipmunkDebugDrawFatSegment(a, b, r, outline, fill);}

static void
ChipmunkDebugDrawPolygonPointer(int count, const cpVect *verts, cpFloat r, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data)
{ChipmunkDebugDrawPolygon(count, verts, r, outline, fill);}

static void
ChipmunkDebugDrawDotPointer(cpFloat size, cpVect pos, cpSpaceDebugColor color, cpDataPointer data)
{ChipmunkDebugDrawDot(size, pos, color);}


// start game
void startGame(GameStruct *game){
    game->gameState = 1;
    game->numLives = 5;
    game->gameScore = 0;
    game->powerupScore = 0;
    game->powerupScoreDisplay = 0;
    game->bumperPowerupState = 0;
    game->ballPowerupState = 0;
    game->waterHeight = 0.0f;
    game->waterPowerupState = 0;
    game->redPowerupOverlay = 0;
    game->bluePowerupOverlay = 0;
    game->slowMotion=0;
    game->slowMotionCounter=0;
    game->leftFlipperState=0;
    game->rightFlipperState=0;
    iceOverlayAlpha=0.0f;
    inputSetScore(game->input,0);
    inputSetGameState(game->input,STATE_GAME);
    inputSetNumBalls(game->input,game->numLives);
}

int main(void){

    // Initialize a struct encoding data about the game.
    GameStruct game;
    game.gameState = 0;


    float walls[128][4] = {
        {0,0,worldWidth,0},
        {0,0,0,worldHeight},
        {worldWidth,0,worldWidth,worldHeight},
        {worldWidth - 6,56,worldWidth - 6,worldHeight},
        {worldWidth - 7,56,worldWidth - 7,worldHeight},
        {worldWidth-6,56,worldWidth-7,56},
        {0,128,19,142},
        {worldWidth - 7,128,worldWidth - 26,142},
        {0,2.1,worldWidth,2.1},
        {40.4,1.6,41.2,4.0},
        {41.2,4.0,65.2,1.6},
        {69.2,16.4,60.4,43.2},
        {60.4,43.2,68.8,55.6},
        {74.8,63.6,83.2,76.0},
        {84.0,56.7,84.0,37.2},
        {70.8,18.4,68,26.8},
        {74.8,37.6,68.8,55.6},
        {82.0,39.2,74.8,63.6},
        {67.400002,146.400009,83.200005,134.199997},
        {16.400000,146.199997,0.600000,134.600006}
    };
    writeCircleWallSegment(walls,20,20,0,90,worldWidth-28.5,30.75,28.75);
    writeCircleWallSegment(walls,40,20,270,360,28.5,30.75,28.75);
    writeCircleWallSegment(walls,60,10,20,110,64.75,35.6,10.15);
    writeCircleWallSegment(walls,70,10,20,110,64.75,35.6,17.50);
    writeCircleWallSegment(walls,80,10,13,110,64.75,35.6,19.50);

    SoundManager *sound = initSound();
    game.sound = sound;


    SetConfigFlags(FLAG_SHOW_LOGO | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Mini Pinball by Chris Dalke!");
    SetTargetFPS(60);

    Texture bgTex = LoadTexture("Resources/Textures/background2.png");
    Texture ballTex = LoadTexture("Resources/Textures/ball.png");
    Texture beachBallTex = LoadTexture("Resources/Textures/beachBall.png");
    Texture trailTex = LoadTexture("Resources/Textures/trail.png");
    Texture bumperTex = LoadTexture("Resources/Textures/bumper.png");
    Texture bumperLightTex = LoadTexture("Resources/Textures/bumperLight.png");
    Texture iceBumperTex = LoadTexture("Resources/Textures/iceBumper.png");
    Texture shockwaveTex = LoadTexture("Resources/Textures/shockwave.png");
    Texture debugTex = LoadTexture("Resources/Textures/debugSmall.png");
    Texture leftFlipperTex = LoadTexture("Resources/Textures/flipperL.png");
    Texture rightFlipperTex = LoadTexture("Resources/Textures/flipperR.png");
    Texture bgMenu = LoadTexture("Resources/Textures/bgMenu.png");
    Texture titleOverlay = LoadTexture("Resources/Textures/titleOverlay.png");
    Texture menuOverlay1 = LoadTexture("Resources/Textures/menuOverlay1.png");
    Texture gameOverOverlay1 = LoadTexture("Resources/Textures/gameOverOverlay1.png");
    Texture gameOverOverlay2 = LoadTexture("Resources/Textures/gameOverOverlay2.png");
    Texture arrowRight = LoadTexture("Resources/Textures/arrowRight.png");
    Texture menuControls = LoadTexture("Resources/Textures/menuControls.png");
    Texture transitionTex = LoadTexture("Resources/Textures/transition.png");
    Texture waterTex = LoadTexture("Resources/Textures/waterTex.png");
    Texture waterOverlayTex = LoadTexture("Resources/Textures/waterOverlayTex.png");
    Texture particleTex = LoadTexture("Resources/Textures/particle.png");
    Texture iceOverlay = LoadTexture("Resources/Textures/iceOverlay.png");
    Texture bumper3 = LoadTexture("Resources/Textures/bumper3.png");
    Texture lowerBumperShock = LoadTexture("Resources/Textures/lowerBumperShock.png");
    Texture redPowerupOverlay = LoadTexture("Resources/Textures/redPowerupOverlay.png");

    Font font1 = LoadFontEx("Resources/Fonts/Avenir-Black.ttf",80,0,0);
    Font font2 = LoadFontEx("Resources/Fonts/Avenir-Black.ttf",120,0,0);

    Shader alphaTestShader = LoadShader(0, FormatText("resources/shaders/glsl%i/alphaTest.fs", GLSL_VERSION));

    Shader swirlShader = LoadShader(0, FormatText("Resources/Shaders/glsl%i/wave.fs", GLSL_VERSION));
    int secondsLoc = GetShaderLocation(swirlShader, "secondes");
	int freqXLoc = GetShaderLocation(swirlShader, "freqX");
	int freqYLoc = GetShaderLocation(swirlShader, "freqY");
	int ampXLoc = GetShaderLocation(swirlShader, "ampX");
	int ampYLoc = GetShaderLocation(swirlShader, "ampY");
	int speedXLoc = GetShaderLocation(swirlShader, "speedX");
	int speedYLoc = GetShaderLocation(swirlShader, "speedY");
	float freqX = 25.0f;
	float freqY = 25.0f;
	float ampX = 5.0f;
	float ampY = 5.0f;
	float speedX = 8.0f;
	float speedY = 8.0f;
    float screenSize[2] = {screenWidth,screenHeight};
	SetShaderValue(swirlShader, GetShaderLocation(swirlShader, "size"), &screenSize, UNIFORM_VEC2);
	SetShaderValue(swirlShader, freqXLoc, &freqX, UNIFORM_FLOAT);
	SetShaderValue(swirlShader, freqYLoc, &freqY, UNIFORM_FLOAT);
	SetShaderValue(swirlShader, ampXLoc, &ampX, UNIFORM_FLOAT);
	SetShaderValue(swirlShader, ampYLoc, &ampY, UNIFORM_FLOAT);
	SetShaderValue(swirlShader, speedXLoc, &speedX, UNIFORM_FLOAT);
	SetShaderValue(swirlShader, speedYLoc, &speedY, UNIFORM_FLOAT);
    float shaderSeconds = 0.0f;


    // Initialize physics simulation
    cpVect gravity = cpv(0,100);
    cpSpace *space = cpSpaceNew();
    game.space = space;
    cpSpaceSetGravity(space,gravity);
    cpFloat timeStep = 1.0/60.0;

    // create walls
    for (int i = 0; i < numWalls; i++){
        cpShape *wall = cpSegmentShapeNew(cpSpaceGetStaticBody(space),cpv(walls[i][0],walls[i][1]),cpv(walls[i][2],walls[i][3]),0);
        cpShapeSetFriction(wall,0.5);
        cpShapeSetElasticity(wall,0.5);
        cpShapeSetCollisionType(wall, COLLISION_WALL);
        cpSpaceAddShape(space,wall);
    }


    // Create bumpers
    const int numBumpers = 14;
    const float bumperSize = 10.0f;
    const float smallBumperSize = 4.0f;
    const float bumperBounciness = 1.8f;
    Bumper* bumpers = malloc(numBumpers * sizeof(Bumper));


    cpShape *bouncer1 = cpSegmentShapeNew(cpSpaceGetStaticBody(space),cpv(14.800000,125.200005),cpv(7.600000,109.200005),0);
    cpShape *bouncer2 = cpSegmentShapeNew(cpSpaceGetStaticBody(space),cpv(75.599998,108.800003),cpv(69.200005,125.200005),0);
    cpShapeSetCollisionType(bouncer1, COLLISION_LEFT_LOWER_BUMPER);
    cpShapeSetCollisionType(bouncer2, COLLISION_RIGHT_LOWER_BUMPER);

    cpShapeSetFriction(bouncer1,0.0);
    cpShapeSetElasticity(bouncer1,1.2f);
    cpSpaceAddShape(space,bouncer1);
    cpShapeSetFriction(bouncer2,0.0);
    cpShapeSetElasticity(bouncer2,1.2f);
    cpSpaceAddShape(space,bouncer2);
    cpShape *bouncerGuard1 = cpSegmentShapeNew(cpSpaceGetStaticBody(space),cpv(7.200000,111.200005),cpv(12.800000,124.400002),0);
    cpShape *bouncerGuard2 = cpSegmentShapeNew(cpSpaceGetStaticBody(space),cpv(71.200005,124.800003),cpv(76.000000,110.800003),0);
    cpShapeSetCollisionType(bouncerGuard1, COLLISION_WALL);
    cpShapeSetCollisionType(bouncerGuard2, COLLISION_WALL);
    cpSpaceAddShape(space,bouncerGuard1);
    cpShapeSetFriction(bouncerGuard1,0.0f);
    cpShapeSetElasticity(bouncerGuard1,0.9f);
    cpSpaceAddShape(space,bouncerGuard2);
    cpShapeSetFriction(bouncerGuard2,0.0f);
    cpShapeSetElasticity(bouncerGuard2,0.9f);


    bumpers[0].body = cpSpaceAddBody(space,cpBodyNewKinematic());
    cpBodySetPosition(bumpers[0].body,cpv(24.9,19.9));
    bumpers[0].shape = cpSpaceAddShape(space,cpCircleShapeNew(bumpers[0].body,bumperSize/2.0f,cpvzero));
    cpShapeSetElasticity(bumpers[0].shape,bumperBounciness);
    cpShapeSetCollisionType(bumpers[0].shape, COLLISION_BUMPER);
    cpShapeSetUserData(bumpers[0].shape,&bumpers[0]);
    bumpers[0].bounceEffect = 0;
    bumpers[0].type = 0;

    bumpers[1].body = cpSpaceAddBody(space,cpBodyNewKinematic());
    cpBodySetPosition(bumpers[1].body,cpv(46.6,17.8));
    bumpers[1].shape = cpSpaceAddShape(space,cpCircleShapeNew(bumpers[1].body,bumperSize/2.0f,cpvzero));
    cpShapeSetElasticity(bumpers[1].shape,bumperBounciness);
    cpShapeSetCollisionType(bumpers[1].shape, COLLISION_BUMPER);
    cpShapeSetUserData(bumpers[1].shape,&bumpers[1]);
    bumpers[1].bounceEffect = 0;
    bumpers[1].type = 0;

    bumpers[2].body = cpSpaceAddBody(space,cpBodyNewKinematic());
    cpBodySetPosition(bumpers[2].body,cpv(38.0,36.4));
    bumpers[2].shape = cpSpaceAddShape(space,cpCircleShapeNew(bumpers[2].body,bumperSize/2.0f,cpvzero));
    cpShapeSetElasticity(bumpers[2].shape,bumperBounciness);
    cpShapeSetCollisionType(bumpers[2].shape, COLLISION_BUMPER);
    cpShapeSetUserData(bumpers[2].shape,&bumpers[2]);
    bumpers[2].bounceEffect = 0;
    bumpers[2].type = 0;

    bumpers[3].body = cpSpaceAddBody(space,cpBodyNewKinematic());
    cpBodySetPosition(bumpers[3].body,cpv(72.200005,23.400000));
    bumpers[3].shape = cpSpaceAddShape(space,cpCircleShapeNew(bumpers[3].body,2.0f,cpvzero));
    cpShapeSetElasticity(bumpers[3].shape,bumperBounciness);
    cpShapeSetCollisionType(bumpers[3].shape, COLLISION_BUMPER);
    cpShapeSetUserData(bumpers[3].shape,&bumpers[3]);
    bumpers[3].bounceEffect = 0;
    bumpers[3].type = 1;

    for (int i = 4; i < 10; i++){
        bumpers[i].body = cpSpaceAddBody(space,cpBodyNewKinematic());
        bumpers[i].shape = cpSpaceAddShape(space,cpCircleShapeNew(bumpers[i].body,2.0f,cpvzero));
        cpShapeSetElasticity(bumpers[i].shape,0);
        cpShapeSetCollisionType(bumpers[i].shape, COLLISION_BUMPER);
        cpShapeSetUserData(bumpers[i].shape,&bumpers[i]);
        bumpers[i].bounceEffect = 0;
        bumpers[i].type = 2;
        bumpers[i].enabled = 1;
    }
    cpBodySetPosition(bumpers[4].body,cpv(63.34,50.88));
    cpBodySetPosition(bumpers[5].body,cpv(77.38,70.96));
    cpBodySetPosition(bumpers[6].body,cpv(15.1,62.04));
    cpBodySetPosition(bumpers[7].body,cpv(18.9,45.3));
    cpBodySetPosition(bumpers[8].body,cpv(61.02,35.36));
    cpBodySetPosition(bumpers[9].body,cpv(65.02,23.02));
    bumpers[4].type = 2;
    bumpers[5].type = 2;
    bumpers[6].type = 2;
    bumpers[7].type = 3;
    bumpers[8].type = 3;
    bumpers[9].type = 3;
    bumpers[4].angle = 90.0+145.2;
    bumpers[5].angle = 90.0+145.2;
    bumpers[6].angle = 90.0+25.7;
    bumpers[7].angle = 90;
    bumpers[8].angle = 90-162;
    bumpers[9].angle = 90-162;

    bumpers[10].body = cpSpaceAddBody(space,cpBodyNewKinematic());
    cpBodySetPosition(bumpers[10].body,cpv(12.2,81.8));
    bumpers[10].shape = cpSpaceAddShape(space,cpCircleShapeNew(bumpers[10].body,smallBumperSize/2.0f,cpvzero));
    cpShapeSetElasticity(bumpers[10].shape,bumperBounciness);
    cpShapeSetCollisionType(bumpers[10].shape, COLLISION_BUMPER);
    cpShapeSetUserData(bumpers[10].shape,&bumpers[10]);
    bumpers[10].bounceEffect = 0;
    bumpers[10].type = 4;

    bumpers[11].body = cpSpaceAddBody(space,cpBodyNewKinematic());
    cpBodySetPosition(bumpers[11].body,cpv(23.8,91.2));
    bumpers[11].shape = cpSpaceAddShape(space,cpCircleShapeNew(bumpers[11].body,smallBumperSize/2.0f,cpvzero));
    cpShapeSetElasticity(bumpers[11].shape,bumperBounciness);
    cpShapeSetCollisionType(bumpers[11].shape, COLLISION_BUMPER);
    cpShapeSetUserData(bumpers[11].shape,&bumpers[11]);
    bumpers[11].bounceEffect = 0;
    bumpers[11].type = 4;

    bumpers[12].body = cpSpaceAddBody(space,cpBodyNewKinematic());
    cpBodySetPosition(bumpers[12].body,cpv(61.2,91.2));
    bumpers[12].shape = cpSpaceAddShape(space,cpCircleShapeNew(bumpers[12].body,smallBumperSize/2.0f,cpvzero));
    cpShapeSetElasticity(bumpers[12].shape,bumperBounciness);
    cpShapeSetCollisionType(bumpers[12].shape, COLLISION_BUMPER);
    cpShapeSetUserData(bumpers[12].shape,&bumpers[12]);
    bumpers[12].bounceEffect = 0;
    bumpers[12].type = 4;

    bumpers[13].body = cpSpaceAddBody(space,cpBodyNewKinematic());
    cpBodySetPosition(bumpers[13].body,cpv(72.599998,81.8));
    bumpers[13].shape = cpSpaceAddShape(space,cpCircleShapeNew(bumpers[13].body,smallBumperSize/2.0f,cpvzero));
    cpShapeSetElasticity(bumpers[13].shape,bumperBounciness);
    cpShapeSetCollisionType(bumpers[13].shape, COLLISION_BUMPER);
    cpShapeSetUserData(bumpers[13].shape,&bumpers[13]);
    bumpers[13].bounceEffect = 0;
    bumpers[13].type = 4;


    //Add collision handler for ball-bumper effect
    cpCollisionHandler *handler = cpSpaceAddCollisionHandler(space,COLLISION_BALL,COLLISION_BUMPER);
    handler->beginFunc = CollisionHandlerBallBumper;

    cpCollisionHandler *ballFlipperHandler = cpSpaceAddCollisionHandler(space,COLLISION_BALL,COLLISION_PADDLE);
    ballFlipperHandler->preSolveFunc = CollisionHandlerBallFlipper;

    cpCollisionHandler *leftLower = cpSpaceAddCollisionHandler(space,COLLISION_BALL,COLLISION_LEFT_LOWER_BUMPER);
    cpCollisionHandler *rightLower = cpSpaceAddCollisionHandler(space,COLLISION_BALL,COLLISION_RIGHT_LOWER_BUMPER);
    leftLower->beginFunc = CollisionHandlerLeftLowerBumper;
    rightLower->beginFunc = CollisionHandlerRightLowerBumper;

    // create one-way door
	cpShape* oneWayShape = cpSpaceAddShape(space, cpSegmentShapeNew(cpSpaceGetStaticBody(space), cpv(69.6,16.6), cpv(73.4,4.6), 0.5f));
	cpShapeSetElasticity(oneWayShape, 0.5f);
	cpShapeSetFriction(oneWayShape, 0.0f);
	cpShapeSetCollisionType(oneWayShape, COLLISION_ONE_WAY);
	cpCollisionHandler *oneWayHandler = cpSpaceAddCollisionHandler(space,COLLISION_BALL,COLLISION_ONE_WAY);
	oneWayHandler->preSolveFunc = CollisionOneWay;


	cpShape* tempShape = cpSpaceAddShape(space, cpSegmentShapeNew(cpSpaceGetStaticBody(space), cpv(7.800000,38.200001), cpv(7.8,49.200001), 1.0f));
	cpShapeSetElasticity(tempShape, 0.5f);
	cpShapeSetFriction(tempShape, 0.5f);
	cpShapeSetCollisionType(tempShape, COLLISION_WALL);

	tempShape = cpSpaceAddShape(space, cpSegmentShapeNew(cpSpaceGetStaticBody(space), cpv(16.000000,38.400002), cpv(16.000000,53.799999), 1.0f));
	cpShapeSetElasticity(tempShape, 0.5f);
	cpShapeSetFriction(tempShape, 0.5f);
	cpShapeSetCollisionType(tempShape, COLLISION_WALL);

	tempShape = cpSpaceAddShape(space, cpSegmentShapeNew(cpSpaceGetStaticBody(space), cpv(16.000000,53.799999), cpv(8.600000,68.800003), 1.0f));
	cpShapeSetElasticity(tempShape, 0.5f);
	cpShapeSetFriction(tempShape, 0.5f);
	cpShapeSetCollisionType(tempShape, COLLISION_WALL);

    // Create left and right flippers
    cpBody* leftFlipperBody = cpBodyNewKinematic();
    cpBody* rightFlipperBody = cpBodyNewKinematic();
    cpBodySetPosition(leftFlipperBody,cpv(19.8,145.45));
    cpBodySetPosition(rightFlipperBody,cpv(63.5,145.45));
    const cpVect flipperPoly[4] = {
        {0,0},
        {flipperWidth,0},
        {flipperWidth,flipperHeight},
        {0,flipperHeight}
    };
    cpShape* leftFlipperShape = cpSpaceAddShape(space,cpPolyShapeNewRaw(leftFlipperBody,4,flipperPoly,0.0f));
    cpShape* rightFlipperShape = cpSpaceAddShape(space,cpPolyShapeNewRaw(rightFlipperBody,4,flipperPoly,0.0f));
    cpShapeSetFriction(leftFlipperShape,0.8);
    cpShapeSetFriction(rightFlipperShape,0.8);
    cpShapeSetElasticity(leftFlipperShape,0.2);
    cpShapeSetElasticity(rightFlipperShape,0.2);
    cpBodySetCenterOfGravity(leftFlipperBody,cpv(flipperHeight/2.0f,flipperHeight/2.0f));
    cpBodySetCenterOfGravity(rightFlipperBody,cpv(flipperHeight/2.0f,flipperHeight/2.0f));
    float flipperSpeed = 900.0f;
    float leftFlipperAngle = 33.0f;
    float rightFlipperAngle = 147.0f;
    float flipperSpeedScalar = 1.0f;

    // Powerup variables
    float powerupFullY = 64.0f;
    float powerupEmptyY = 104.4f;
    float powerupTargetScore = 5000.0f;

    //create balls array
    Ball* balls = malloc(maxBalls * sizeof(Ball));
    game.balls = balls;
    game.numBalls = 0;
    for (int i = 0; i < maxBalls; i++){
        balls[i].active = 0;
    }

    // Setup render texture for special ball effect
    RenderTexture2D renderTarget = LoadRenderTexture(screenWidth, screenHeight);


    // Setup debug draw options
    cpSpaceDebugDrawOptions drawOptions = {
    	ChipmunkDebugDrawCirclePointer,
    	ChipmunkDebugDrawSegmentPointer,
    	ChipmunkDebugDrawFatSegmentPointer,
    	ChipmunkDebugDrawPolygonPointer,
    	ChipmunkDebugDrawDotPointer,
    	(cpSpaceDebugDrawFlags)(CP_SPACE_DEBUG_DRAW_SHAPES | CP_SPACE_DEBUG_DRAW_CONSTRAINTS | CP_SPACE_DEBUG_DRAW_COLLISION_POINTS),
    	{0xEE/255.0f, 0xE8/255.0f, 0xD5/255.0f, 1.0f}, // Outline color
    	ChipmunkDebugGetColorForShape,
    	{0.0f, 0.75f, 0.0f, 1.0f}, // Constraint color
    	{1.0f, 0.0f, 0.0f, 1.0f}, // Collision point color
    	NULL,
    };

    // Menu setup
    MenuPinball* menuPinballs = malloc(32 * sizeof(MenuPinball));
    // Initialize pinballs
    for (int i = 0; i < 32; i++){
        menuPinballs[i].px = -100;
        menuPinballs[i].py = (rand() % screenHeight);
        menuPinballs[i].vx = 0;
        menuPinballs[i].vy = 0;
    }

    // Setup input
    InputManager *input = inputInit();
    game.input = input;

    // Setup score system
    ScoreHelper *scores = initScores();

    // Setup timestepping system
    int timestep = 1000.0/60.0;
    long long accumulatedTime = 0;
    long long startTime = millis();
    long long endTime = millis();
    long long elapsedTimeStart = millis();

    game.transitionState = 0;
    game.transitionAlpha = 0;
    game.transitionTarget = TRANSITION_TO_MENU;

    game.menuState = 0;

    //game.gameState = 2;
    game.nameSelectIndex = 0;
    game.nameSelectDone = 0;

    multiballOverlayY = 20 + worldHeight;

    game.gameState = 5;

    char tempString[128];
    char nameString[6];
    sprintf(nameString,"     ");

    inputSetGameState(input,STATE_MENU);

    while (!WindowShouldClose()){
        endTime = millis();
        accumulatedTime += (endTime - startTime);
        startTime = millis();
        shaderSeconds += GetFrameTime() / 2.0f;
        SetShaderValue(swirlShader, secondsLoc, &shaderSeconds, UNIFORM_FLOAT);

        float mouseX = GetMouseX();
        float mouseY = GetMouseY();

        // Poll input
        inputUpdate(input);

        game.slowMotionFactor = slowMotionFactor;

        // STEP SIMULATION AT FIXED RATE
        while (accumulatedTime > timestep){
            accumulatedTime -= timestep;

            updateSound(sound,&game);

            if (game.transitionState == 1){
                // TRANSITION OUT
                game.transitionAlpha += 15;
                if (game.transitionAlpha >= 255){
                    game.transitionState = 2;
                    game.transitionAlpha = 255;
                    game.transitionDelay = 0;
                }
            } else if (game.transitionState == 2){
                // HANDLE LOAD
                switch (game.transitionTarget){
                    case TRANSITION_TO_GAME: {
                        startGame(&game);
                        bumpers[4].enabled = 1;
                        bumpers[5].enabled = 1;
                        bumpers[6].enabled = 1;
                        bumpers[7].enabled = 1;
                        bumpers[8].enabled = 1;
                        bumpers[9].enabled = 1;
                        bumpers[10].enabled = 0;
                        bumpers[11].enabled = 0;
                        bumpers[12].enabled = 0;
                        bumpers[13].enabled = 0;
                        break;
                    }
                    case TRANSITION_TO_MENU: {
                        game.gameState = 0;
                        break;
                    }
                    case TRANSITION_GAME_OVER: {
                        game.gameState = 2;
                        game.nameSelectIndex = 0;
                        game.nameSelectDone = 0;
                        break;
                    }
                }
                game.transitionDelay++;
                if (game.transitionDelay > 10){
                    game.transitionState = 3;
                }
            } else if (game.transitionState == 3){
                //TRANSITION IN
                game.transitionAlpha -= 15;
                if (game.transitionAlpha <= 0){
                    game.transitionState = 0;
                    game.transitionAlpha = 0;
                }
            } else {
                game.transitionAlpha = 0;
            }
            if (game.gameState == 5){
                //transition from raylib title to menu
                if (game.transitionState == 0){
                    game.transitionState = 1;
                    game.transitionTarget = TRANSITION_TO_MENU;
                }
            }

            // Update menu pinballs
            for (int i = 0; i < 16; i++){
                menuPinballs[i].px += menuPinballs[i].vx;
                menuPinballs[i].py += menuPinballs[i].vy;
                menuPinballs[i].vy += 0.1f;
                if (menuPinballs[i].py > screenHeight + 20){
                    menuPinballs[i].px = 228;
                    menuPinballs[i].py = 126;
                    menuPinballs[i].vx = ((rand() % 40) / 10.0f) - 2.0f;
                    menuPinballs[i].vy = ((rand() % 50) / -10.0f);
                }
            }
            if (game.gameState == 0){
                if (inputCenterPressed(input)){
                    game.transitionState = 1;
                    game.transitionTarget = TRANSITION_TO_GAME;
                    playClick(sound);
                }
                if (inputLeftPressed(input))  {
                    playClick(sound);
                    game.menuState = 1;
                }
                if (inputRightPressed(input)) {
                    playClick(sound);
                    game.menuState = 0;
                }
            }
            float effectiveTimestep = (timeStep) * slowMotionFactor;
            if (game.gameState == 1){
                // Game
                cpSpaceStep(space, effectiveTimestep / 3.0f);
                cpSpaceStep(space, effectiveTimestep / 3.0f);
                cpSpaceStep(space, effectiveTimestep / 3.0f);

                if (game.oldGameScore != game.gameScore){
                    inputSetScore(input,game.gameScore);
                    game.oldGameScore = game.gameScore;
                }
                // Check powerups before dispensing balls
                if (game.ballPowerupState == 0 && !bumpers[7].enabled && !bumpers[8].enabled && !bumpers[9].enabled){
                    // spawn balls
                    for (int i =0; i < 3; i++){
                        addBall(&game,89.5 - ballSize / 2,160 - (i * ballSize),0,-220,1);
                    }
                    playBluePowerupSound(sound);
                    game.bluePowerupOverlay = 1.0f;
                    game.ballPowerupState = -1;
                    game.gameScore += 500;
                    if (game.waterPowerupState == 0){
                        game.powerupScore += 500;
                    }
                } else if (game.ballPowerupState == -1){
                    // Check if there are no balls left. Then powerup resets and bumpers reset.
                    if (game.numBalls == 0){
                        game.ballPowerupState = 0;
                        bumpers[7].enabled = 1;
                        bumpers[8].enabled = 1;
                        bumpers[9].enabled = 1;
                    }
                }

                if (game.bumperPowerupState ==0 && !bumpers[4].enabled && !bumpers[5].enabled && !bumpers[6].enabled){
                    // spawn bumpers
                    game.bumperPowerupState = -1;
                    bumpers[10].enabled = 1;
                    bumpers[11].enabled = 1;
                    bumpers[12].enabled = 1;
                    bumpers[13].enabled = 1;
                    playRedPowerupSound(sound);
                    game.redPowerupOverlay = 1.0f;
                    game.gameScore += 500;
                    if (game.waterPowerupState == 0){
                        game.powerupScore += 500;
                    }
                } else if (game.bumperPowerupState == -1){
                    if (!bumpers[10].enabled && !bumpers[11].enabled && !bumpers[12].enabled && !bumpers[13].enabled){
                        game.bumperPowerupState = 0;
                        bumpers[4].enabled = 1;
                        bumpers[5].enabled = 1;
                        bumpers[6].enabled = 1;
                        game.redPowerupOverlay = 1.0f;
                    }
                }

                if (game.numBalls == 0){
                    if (game.numLives >= 1){
                        if (inputCenterPressed(input)){
                            addBall(&game,89.5 - ballSize / 2,160,0,-220,0);
                        }
                    } else {
                        // game over condition
                        if (game.transitionState == 0){
                            game.transitionState = 1;
                            game.transitionTarget = TRANSITION_GAME_OVER;
                            inputSetGameState(input,STATE_GAME_OVER);
                        }
                    }
                }

                if (IsMouseButtonPressed(0)){
                    addBall(&game,(mouseX) * screenToWorld,(mouseY) * screenToWorld,0,0,1);
                }

                float oldAngleLeft = leftFlipperAngle;
                float oldAngleRight = rightFlipperAngle;
                float targetAngleLeft = 0.0f;
                float targetAngleRight = 0.0f;
                if (inputLeft(input)){
                    if (game.leftFlipperState == 0){
                        playFlipper(sound);
                        game.leftFlipperState = 1;
                    }
                    targetAngleLeft = -33.0f - 10.0f;
                    leftFlipperAngle -= (flipperSpeed * effectiveTimestep);
                    if (leftFlipperAngle < targetAngleLeft){
                        leftFlipperAngle = targetAngleLeft;
                    }
                } else {
                    if (game.leftFlipperState == 1){
                        playFlipper(sound);
                    }
                    game.leftFlipperState = 0;
                    targetAngleLeft = 33.0f;
                    leftFlipperAngle += (flipperSpeed * effectiveTimestep);
                    if (leftFlipperAngle > targetAngleLeft){
                        leftFlipperAngle = targetAngleLeft;
                    }
                }
                if (inputRight(input)){
                    if (game.rightFlipperState == 0){
                        playFlipper(sound);
                        game.rightFlipperState = 1;
                    }
                    targetAngleRight = 213.0f + 10.0f;
                    rightFlipperAngle += (flipperSpeed * effectiveTimestep);
                    if (rightFlipperAngle > targetAngleRight){
                        rightFlipperAngle = targetAngleRight;
                    }
                } else {
                    if (game.rightFlipperState == 1){
                        playFlipper(sound);
                    }
                    game.rightFlipperState = 0;
                    targetAngleRight = 147.0f;
                    rightFlipperAngle -= (flipperSpeed * effectiveTimestep);
                    if (rightFlipperAngle < targetAngleRight){
                        rightFlipperAngle = targetAngleRight;
                    }
                }

                float deltaAngularVelocityLeft = ((leftFlipperAngle * DEG_TO_RAD) - (oldAngleLeft * DEG_TO_RAD)) / effectiveTimestep;
                float deltaAngularVelocityRight = ((rightFlipperAngle * DEG_TO_RAD) - (oldAngleRight * DEG_TO_RAD)) / effectiveTimestep;
                cpBodySetAngle(leftFlipperBody,leftFlipperAngle * DEG_TO_RAD);
                cpBodySetAngle(rightFlipperBody,rightFlipperAngle * DEG_TO_RAD);
                cpBodySetAngularVelocity(leftFlipperBody,deltaAngularVelocityLeft * flipperSpeedScalar);
                cpBodySetAngularVelocity(rightFlipperBody,deltaAngularVelocityRight * flipperSpeedScalar);
                cpSpaceReindexShapesForBody(space,leftFlipperBody);
                cpSpaceReindexShapesForBody(space,rightFlipperBody);

                // Check if any balls have fallen outside the screen
                // Remove them if they have.
                // Check if any balls are standing still for too long and remove.
                for (int i = 0; i < maxBalls; i++){
                    if (balls[i].active == 1){
                        cpVect pos = cpBodyGetPosition(balls[i].body);
                        cpVect vel = cpBodyGetVelocity(balls[i].body);
                        if (cpvlengthsq(vel)<0.01f){
                            balls[i].killCounter++;
                        } else {
                            balls[i].killCounter=0;
                        }
                        // Reset kill counter near flippers
                        if (pos.y > 118){
                            balls[i].killCounter=0;
                        }
                        if (pos.y > 170+ballSize || balls[i].killCounter > 100){
                            balls[i].active = 0;
                            cpSpaceRemoveShape(game.space,balls[i].shape);
                            cpSpaceRemoveBody(game.space,balls[i].body);
                            cpShapeFree(balls[i].shape);
                            cpBodyFree(balls[i].body);
                            game.numBalls--;
                            //Check number of lives and send to score if necessary
                            if (game.numBalls == 0){
                                if (game.numLives >= 1){
                                    game.numLives -= 1;
                                    inputSetNumBalls(input,game.numLives);
                                }
                            }
                        }
                    }
                }

                //Update ball trails
                for (int i = 0; i < maxBalls; i++){
                    if (balls[i].active == 1){
                        cpVect pos = cpBodyGetPosition(balls[i].body);
                        balls[i].locationHistoryX[balls[i].trailStartIndex] = pos.x;
                        balls[i].locationHistoryY[balls[i].trailStartIndex] = pos.y;
                        balls[i].trailStartIndex = (balls[i].trailStartIndex + 1) % 16;
                    }
                }

                //handler lower bumpers
                if (leftLowerBumperAnim > 0.0f){
                    leftLowerBumperAnim -= 0.05f;
                    if (leftLowerBumperAnim < 0.0f){
                        leftLowerBumperAnim = 0.0f;
                    }
                }
                if (rightLowerBumperAnim > 0.0f){
                    rightLowerBumperAnim -= 0.05f;
                    if (rightLowerBumperAnim < 0.0f){
                        rightLowerBumperAnim = 0.0f;
                    }
                }

                //update ice overlay
                if (game.slowMotion == 1){
                    slowMotionFactor = 0.3f;
                    iceOverlayAlpha += 0.01f;
                    if (iceOverlayAlpha >= 1.0f){
                        iceOverlayAlpha = 1.0f;
                    }
                } else {
                    if (slowMotionFactor < 1.0f){
                        slowMotionFactor += 0.05f;
                        if (slowMotionFactor > 1.0f){
                            slowMotionFactor = 1.0f;
                        }
                    }
                    iceOverlayAlpha -= 0.01f;
                    if (iceOverlayAlpha <= 0.0f){
                        iceOverlayAlpha = 0.0f;
                    }
                }
                if (game.slowMotionCounter > 0){
                    game.slowMotionCounter--;
                    if (game.slowMotionCounter <= 0){
                        game.slowMotion = 0;
                        playSpeedupSound(sound);
                    }
                }

                // Update red and blue powerup overlays
                if (game.redPowerupOverlay > 0.0f){
                    game.redPowerupOverlay -= 0.02f * slowMotionFactor;
                    if (game.redPowerupOverlay <= 0.0f){
                        game.redPowerupOverlay = 0.0f;
                    }
                }
                if (game.bluePowerupOverlay > 0.0f){
                    game.bluePowerupOverlay -= 0.04f * slowMotionFactor;
                    if (game.bluePowerupOverlay <= 0.0f){
                        game.bluePowerupOverlay = 0.0f;
                    }
                }

                // Update bumper
                for (int i = 0; i < numBumpers; i++){
                    bumpers[i].bounceEffect *= 0.94;
                    if (bumpers[i].enabled){
                        bumpers[i].enabledSize += 0.1f;
                        if (bumpers[i].enabledSize > 1.0f){
                            bumpers[i].enabledSize = 1.0f;
                        }
                    } else {
                        bumpers[i].enabledSize -= 0.1f;
                        if (bumpers[i].enabledSize < 0.0f){
                            bumpers[i].enabledSize = 0.0f;
                        }
                    }
                }

                // Update powerup score display
                if (game.powerupScoreDisplay < game.powerupScore){
                    game.powerupScoreDisplay += 10;
                    if (game.powerupScoreDisplay > game.powerupScore){
                        game.powerupScoreDisplay = game.powerupScore;
                    }
                } else if (game.powerupScoreDisplay > game.powerupScore){
                    game.powerupScoreDisplay -= 20;
                    if (game.powerupScoreDisplay < game.powerupScore){
                        game.powerupScoreDisplay = game.powerupScore;
                    }
                }
                if (game.powerupScoreDisplay < 0){
                    game.powerupScoreDisplay = 0;
                }
                // If the powerup is full, dispense powerup
                if (game.powerupScoreDisplay >= powerupTargetScore){
                    game.powerupScore = 0;
                    game.waterHeightTarget = 0.5f;
                    game.waterHeightTimer = 400.0f;
                    game.waterPowerupState = 1;
                    playWater(sound);

                    game.gameScore += 1000;
                    if (game.waterPowerupState == 0){
                        game.powerupScore += 1000;
                    }
                }


                if (game.waterPowerupState == 1){
                    game.waterHeight += 0.006f * slowMotionFactor;
                    if (game.waterHeight > game.waterHeightTarget){
                        game.waterHeight = game.waterHeightTarget;
                    }
                } else if (game.waterPowerupState == 2) {
                    game.waterHeight -= 0.0005f * slowMotionFactor;
                    if (game.waterHeight < 0.0f){
                        game.waterHeight = 0.0f;
                        game.waterPowerupState = 0;
                    }
                }

                if (game.waterHeightTimer > 0.0f){
                    game.waterHeightTimer -= 1.0f * slowMotionFactor;
                    if (game.waterHeightTimer <= 0.0f){
                        game.waterHeightTarget = 0.0f;
                        game.waterPowerupState = 2;
                        printf("water timer runout\n");
                    }
                }

                // If water height powerup active, apply buoyancy forces to balls.
                if (game.waterHeight > 0){
                    float waterY = worldHeight * (1.0f - game.waterHeight);

                    for (int i = 0; i < maxBalls; i++){
                        if (balls[i].active == 1){
                            cpVect pos = cpBodyGetPosition(balls[i].body);
                            cpVect vel = cpBodyGetVelocity(balls[i].body);
                            if (pos.y > waterY){
                                float distUnderwater = fabs(waterY - pos.y);
                                float bVely = -200.0f + -(distUnderwater * 40.0f);
                                cpBodyApplyForceAtLocalPoint (balls[i].body, cpv(0,bVely), cpvzero);
                                // Apply special forces for flipper
                                float flipperForce = -1000.0f;
                                if (pos.x <= worldWidth / 2.0f && fabsf(deltaAngularVelocityLeft) > 0){
                                    cpBodyApplyForceAtLocalPoint (balls[i].body, cpv(0,flipperForce), cpvzero);
                                }
                                if (pos.x >= worldWidth / 2.0f && fabsf(deltaAngularVelocityRight) > 0){
                                    cpBodyApplyForceAtLocalPoint (balls[i].body, cpv(0,flipperForce), cpvzero);
                                }
                                if (balls[i].underwaterState == 0){
                                    playWaterSplash(sound);
                                    balls[i].underwaterState = 1;
                                }
                            } else {
                                balls[i].underwaterState = 0;
                            }
                        }
                    }

                }
            }
            if (game.gameState == 2){
                // Game over
                if (game.nameSelectDone == 0){
                    if (inputRightPressed(input)){
                        playClick(sound);
                        game.nameSelectIndex++;
                        if (game.nameSelectIndex > 5){
                            game.nameSelectIndex = 0;
                        }
                    }
                    if (inputLeftPressed(input)){
                        playClick(sound);
                        game.nameSelectIndex--;
                        if (game.nameSelectIndex < 0){
                            game.nameSelectIndex = 5;
                        }
                    }
                    if (inputCenterPressed(input)){
                        playClick(sound);
                        if (game.nameSelectIndex == 5){
                            // Name selection done
                            // Submit score and start transition to menu.
                            game.nameSelectDone = 1;
                            game.transitionState = 1;
                            game.transitionTarget = TRANSITION_TO_MENU;
                            submitScore(scores,nameString,game.gameScore);
                            printf("Game Over. score: %ld\n",game.gameScore);
                            inputSetGameState(input,STATE_MENU);
                        } else {
                            if (game.nameSelectIndex > 0){
                                while (game.nameSelectIndex-1 >= 0 && nameString[game.nameSelectIndex-1] == 32){
                                    game.nameSelectIndex--;
                                }
                            }
                            if (nameString[game.nameSelectIndex] < 65 || nameString[game.nameSelectIndex] > 90){
                                nameString[game.nameSelectIndex] = 65;
                            } else {
                                nameString[game.nameSelectIndex] = (nameString[game.nameSelectIndex] + 1);
                                if (nameString[game.nameSelectIndex] > 90){
                                    nameString[game.nameSelectIndex] = 32;
                                } else if (nameString[game.nameSelectIndex] < 65){
                                    nameString[game.nameSelectIndex] = 90;
                                }
                            }
                        }
                    }
                }
            }
        }

        // RENDER AT SPEED GOVERNED BY RAYLIB
        BeginDrawing();
        if (game.gameState == 0){
            // Menu
            ClearBackground((Color){255,183,0,255});
            float timeFactor = (millis() - elapsedTimeStart) / 1000.0f;
            float xOffset = sin(timeFactor) * 50.0f;
            float yOffset = cos(timeFactor) * 50.0f;
            float angle = sin(timeFactor * 2) * 20 + cos(timeFactor / 3) * 25;
            float width = screenWidth * 3;
            float height = screenHeight * 3;
			BeginShaderMode(swirlShader);
            DrawTexturePro(bgMenu,(Rectangle){0,0,bgMenu.width,bgMenu.height},(Rectangle){xOffset + screenWidth/2,yOffset + screenWidth/2,width,height},(Vector2){width/2,height/2},angle,WHITE);
            EndShaderMode();

            // Render pinballs
            for (int i = 0; i < 16; i++){
                DrawTexturePro(ballTex,(Rectangle){0,0,ballTex.width,ballTex.height},(Rectangle){menuPinballs[i].px,menuPinballs[i].py,30,30},(Vector2){0,0},0,(Color){255,183,0,255});
            }

            DrawTexturePro(menuOverlay1,(Rectangle){0,0,titleOverlay.width,titleOverlay.height},(Rectangle){0,0,screenWidth,screenHeight},(Vector2){0,0},0,WHITE);
            DrawTexturePro(titleOverlay,(Rectangle){0,0,titleOverlay.width,titleOverlay.height},(Rectangle){0,12 + sin(timeFactor)*5.0f,screenWidth,screenHeight},(Vector2){0,0},0,WHITE);

            if (game.menuState == 0){
                DrawTextEx(font1, "Top Scores", (Vector2){153,329}, 36.0, 1.0, WHITE);
                float y = 362;
                for (int i = 1; i <= 10; i++){
                    ScoreObject *score = getRankedScore(scores,i);
                    if (score != NULL){
                        sprintf(tempString,"%d)",i);
                        DrawTextEx(font1, tempString, (Vector2){66 - MeasureTextEx(font1, tempString, 27.0, 1.0).x,y}, 27.0, 1.0, WHITE);
                        sprintf(tempString,"%s",score->scoreName);
                        DrawTextEx(font1, tempString, (Vector2){75,y}, 27.0, 1.0, WHITE);
                        float scoreNameWidth = MeasureTextEx(font1, tempString, 27.0, 1.0).x;
                        sprintf(tempString,"%d",score->scoreValue);
                        float scoreValueWidth = MeasureTextEx(font1, tempString, 27.0, 1.0).x;
                        DrawTextEx(font1, tempString, (Vector2){404 - scoreValueWidth,y}, 27.0, 1.0, WHITE);
                        float lineY = y + 27.0 / 2.0f - 1.0f;
                        DrawLineEx((Vector2){75 + (scoreNameWidth + 10),lineY}, (Vector2){404 - (scoreValueWidth + 10),lineY}, 2, (Color){255,255,255,50});
                    } else {
                        sprintf(tempString,"%d)",i);
                        DrawTextEx(font1, tempString, (Vector2){66 - MeasureTextEx(font1, tempString, 27.0, 1.0).x,y}, 27.0, 1.0, GRAY);
                        DrawTextEx(font1, "No Score", (Vector2){75,y}, 27.0, 1.0, GRAY);
                    }
                    y += (27.0 * 0.8) + 2;
                }
            } else if (game.menuState == 1){
                DrawTexturePro(menuControls,(Rectangle){0,0,menuControls.width,menuControls.height},(Rectangle){26,320,menuControls.width/2,menuControls.height/2},(Vector2){0,0},0,WHITE);
            }

        }
        if (game.gameState == 1){
            // Game
            ClearBackground((Color){40,1,42,255});

            // Draw powerup status under game background
            float powerupProportion = game.powerupScoreDisplay / powerupTargetScore;
            if (powerupProportion > 1.0f){ powerupProportion = 1.0f; }
            float powerupHeight = (powerupEmptyY - powerupFullY) * 2;
            float powerupY = powerupFullY - (powerupProportion * powerupHeight / 2.0f);
            BeginShaderMode(swirlShader);
            DrawTexturePro(waterTex,(Rectangle){0,0,waterTex.width,waterTex.height},(Rectangle){30 * worldToScreen,powerupY* worldToScreen,powerupHeight* worldToScreen,powerupHeight* worldToScreen},(Vector2){0,0},0,WHITE);
            EndShaderMode();

            DrawTexturePro(bgTex,(Rectangle){0,0,bgTex.width,bgTex.height},(Rectangle){0,0,screenWidth,screenHeight},(Vector2){0,0},0,WHITE);

            if (game.redPowerupOverlay > 0.0f){
                DrawTexturePro(redPowerupOverlay,(Rectangle){0,0,bgTex.width,bgTex.height},(Rectangle){0,0,screenWidth,screenHeight},(Vector2){0,0},0,(Color){255,255,255,40.0*game.redPowerupOverlay});
            }

            // render bumpers which belong behind balls.
            for (int i = 0; i < numBumpers; i++){
                cpVect pos = cpBodyGetPosition(bumpers[i].body);
                if (bumpers[i].type == 2 || bumpers[i].type == 3){
                    float width = 8.0f;
                    float height = 2.0f;
                    Color bumperColor = (Color){0,0,0,80};
                    if (bumpers[i].enabled == 0){
                        width = 8.0f;
                        height = 1.5f;
                    } else {
                        if (bumpers[i].type == 2){
                            bumperColor = RED;
                        } else if (bumpers[i].type == 3){
                            bumperColor = BLUE;
                        }
                    }
                    DrawTexturePro(bumper3,(Rectangle){0,0,bumper3.width,bumper3.height},(Rectangle){pos.x * worldToScreen,pos.y * worldToScreen,width * worldToScreen,height * worldToScreen},(Vector2){(width / 2.0) * worldToScreen,(height / 2.0) * worldToScreen},bumpers[i].angle,bumperColor);
                }
            }

            // Render ball trails
            for (int i = 0; i < maxBalls; i++){
                if (balls[i].active == 1){
                    for (int ii = 1; ii <= 16; ii++){
                        int index = (balls[i].trailStartIndex + ii - 1);
                        if (index >= 16){ index -= 16; }
                        float trailSize = ballSize * sqrt(ii/16.0f);
                        Color ballColor = (Color){255,183,0,255};
                        if (balls[i].type == 1){ ballColor = BLUE; }
                        if (game.slowMotion == 1){ ballColor = WHITE; }
                        DrawTexturePro(trailTex,(Rectangle){0,0,trailTex.width,trailTex.height},(Rectangle){balls[i].locationHistoryX[index] * worldToScreen,balls[i].locationHistoryY[index] * worldToScreen,trailSize * worldToScreen,trailSize * worldToScreen},(Vector2){(trailSize / 2.0) * worldToScreen,(trailSize / 2.0) * worldToScreen},0,ballColor);

                    }
                }
            }

            //render balls
            for (int i = 0; i < maxBalls; i++){
                if (balls[i].active == 1){
                    cpVect pos = cpBodyGetPosition(balls[i].body);
                    Color ballColor = (Color){255,183,0,255};
                    if (balls[i].type == 1){ ballColor = BLUE; }
                    if (game.slowMotion == 1){ ballColor = WHITE; }
                    DrawTexturePro(ballTex,(Rectangle){0,0,ballTex.width,ballTex.height},(Rectangle){pos.x * worldToScreen,pos.y * worldToScreen,ballSize * worldToScreen,ballSize * worldToScreen},(Vector2){(ballSize / 2.0) * worldToScreen,(ballSize / 2.0) * worldToScreen},0,ballColor);
                }
            }

            // Render bumpers which belong in front of balls
            for (int i = 0; i < numBumpers; i++){
                cpVect pos = cpBodyGetPosition(bumpers[i].body);
                if (bumpers[i].type == 0){
                    float bounceScale = 0.2f;
                    float width = bumperSize + cos(millis() / 20.0) * bumpers[i].bounceEffect * bounceScale;
                    float height = bumperSize + sin(millis() / 20.0) * bumpers[i].bounceEffect * bounceScale;
                    float shockSize = (bumperSize * bumpers[i].bounceEffect) * 0.15f;
                    DrawTexturePro(shockwaveTex,(Rectangle){0,0,shockwaveTex.width,shockwaveTex.height},(Rectangle){pos.x * worldToScreen,pos.y * worldToScreen,shockSize * worldToScreen,shockSize * worldToScreen},(Vector2){shockSize/2 * worldToScreen,shockSize/2 * worldToScreen},0,WHITE);
                    DrawTexturePro(bumperTex,(Rectangle){0,0,bumperTex.width,bumperTex.height},(Rectangle){pos.x * worldToScreen,pos.y * worldToScreen,width * worldToScreen,height * worldToScreen},(Vector2){(width / 2.0) * worldToScreen,(height / 2.0) * worldToScreen},0,WHITE);
                } else if (bumpers[i].type == 1){
                    float width = 6.0f;
                    float height = 6.0f;
                    float shockPercent = (bumpers[i].bounceEffect) / 20.0f;
                    float shockSize = shockPercent * 20.0f;
                    float angle = sin(shaderSeconds) * 50.0f;
                    DrawTexturePro(iceBumperTex,(Rectangle){0,0,iceBumperTex.width,iceBumperTex.height},(Rectangle){pos.x * worldToScreen,pos.y * worldToScreen,width * worldToScreen,height * worldToScreen},(Vector2){(width / 2.0) * worldToScreen,(height / 2.0) * worldToScreen},angle,WHITE);
                    DrawTexturePro(trailTex,(Rectangle){0,0,trailTex.width,trailTex.height},(Rectangle){pos.x * worldToScreen,pos.y * worldToScreen,shockSize * worldToScreen,shockSize * worldToScreen},(Vector2){(shockSize / 2.0) * worldToScreen,(shockSize / 2.0) * worldToScreen},0,(Color){255,255,255,255 * shockPercent});

                } else if (bumpers[i].type == 4){
                    float bounceScale = 0.2f;
                    float width = smallBumperSize + cos(millis() / 20.0) * bumpers[i].bounceEffect * bounceScale;
                    float height = smallBumperSize + sin(millis() / 20.0) * bumpers[i].bounceEffect * bounceScale;
                    width *= bumpers[i].enabledSize;
                    height *= bumpers[i].enabledSize;
                    float shockSize = (smallBumperSize * bumpers[i].bounceEffect) * 0.15f;
                    shockSize *= bumpers[i].enabledSize;
                    DrawTexturePro(shockwaveTex,(Rectangle){0,0,shockwaveTex.width,shockwaveTex.height},(Rectangle){pos.x * worldToScreen,pos.y * worldToScreen,shockSize * worldToScreen,shockSize * worldToScreen},(Vector2){shockSize/2 * worldToScreen,shockSize/2 * worldToScreen},0,RED);
                    DrawTexturePro(bumperLightTex,(Rectangle){0,0,bumperTex.width,bumperTex.height},(Rectangle){pos.x * worldToScreen,pos.y * worldToScreen,width * worldToScreen,height * worldToScreen},(Vector2){(width / 2.0) * worldToScreen,(height / 2.0) * worldToScreen},0,RED);
                    printf("bumper %d: %f %f %f\n",i,width,height,bumpers[i].enabledSize);
                }
            }

            //render lower bumpers
            if (leftLowerBumperAnim > 0.0f){
                float percent = 1.0f - leftLowerBumperAnim;
                float x = 10.0f;
                float y = 117.2f;
                float width = 8.0f+ (2.0f * percent);
                float height = 18.0f + (4.0f * percent);
                float angle = -24.0f + sin(shaderSeconds * 100.0f) * 10.0f;
                DrawTexturePro(lowerBumperShock,(Rectangle){0,0,lowerBumperShock.width,lowerBumperShock.height},(Rectangle){x * worldToScreen,y * worldToScreen,width * worldToScreen,height * worldToScreen},(Vector2){(width / 2.0) * worldToScreen,(height / 2.0) * worldToScreen},angle,(Color){255,255,255,255* (1.0f -percent)});
            }
            if (rightLowerBumperAnim > 0.0f){
                float percent = 1.0f - rightLowerBumperAnim;
                float x = 73.2f;
                float y = 117.2f;
                float width = 8.0f+ (2.0f * percent);
                float height = 18.0f + (4.0f * percent);
                float angle = 24.0f - sin(shaderSeconds * 100.0f) * 10.0f;
                DrawTexturePro(lowerBumperShock,(Rectangle){0,0,lowerBumperShock.width,lowerBumperShock.height},(Rectangle){x * worldToScreen,y * worldToScreen,width * worldToScreen,height * worldToScreen},(Vector2){(width / 2.0) * worldToScreen,(height / 2.0) * worldToScreen},angle,(Color){255,255,255,255* (1.0f -percent)});

            }

            // Render left flipper
            cpVect pos = cpBodyGetPosition(leftFlipperBody);
            cpFloat angle = cpBodyGetAngle(leftFlipperBody);
            DrawTexturePro(leftFlipperTex,(Rectangle){0,0,leftFlipperTex.width,leftFlipperTex.height},(Rectangle){pos.x * worldToScreen,pos.y * worldToScreen,flipperWidth * worldToScreen,flipperHeight * worldToScreen},(Vector2){0 * worldToScreen,0 * worldToScreen},(angle * RAD_TO_DEG),WHITE);

            // Render right flipper
            pos = cpBodyGetPosition(rightFlipperBody);
            angle = cpBodyGetAngle(rightFlipperBody);
            DrawTexturePro(rightFlipperTex,(Rectangle){0,0,rightFlipperTex.width,rightFlipperTex.height},(Rectangle){pos.x * worldToScreen,pos.y * worldToScreen,flipperWidth * worldToScreen,flipperHeight * worldToScreen},(Vector2){0 * worldToScreen,0 * worldToScreen},(angle * RAD_TO_DEG),WHITE);

            // Render score
            //sprintf(tempString,"%ld",game.gameScore);
            //DrawTextEx(font1, tempString, (Vector2){10,screenHeight - 35}, 30, 1.0, WHITE);

            // Render water powerup when active
            if (game.waterPowerupState > 0){
                float waterY = screenHeight * (1.0f - game.waterHeight);
                BeginShaderMode(swirlShader);
                DrawTexturePro(waterOverlayTex,(Rectangle){0,0,waterOverlayTex.width,waterOverlayTex.height},(Rectangle){0,waterY-30.0f,screenWidth,screenHeight},(Vector2){0,0},0,(Color){255,255,255,100});
                EndShaderMode();
            }

            if (game.bluePowerupOverlay > 0.0f){
                DrawRectangle(0,0,screenWidth,screenHeight,(Color){128,128,255,128*game.bluePowerupOverlay});
            }

            // Render ice powerup when active
            if (iceOverlayAlpha > 0.0f){
                BeginBlendMode(BLEND_ADDITIVE);
                DrawTexturePro(iceOverlay,(Rectangle){0,0,iceOverlay.width,iceOverlay.height},(Rectangle){0,0,screenWidth,screenHeight},(Vector2){0,0},0,(Color){255,255,255,128*iceOverlayAlpha});
                EndBlendMode();
            }

            if (game.numBalls == 0 && game.numLives > 0){
                DrawRectangleRounded((Rectangle){108,600,screenWidth-238,80},0.1,16,(Color){0,0,0,100});
                DrawRectangleRounded((Rectangle){112,604,screenWidth-242,76},0.1,16,(Color){0,0,0,100});

                if (game.numLives == 5){
                    DrawTextEx(font1, "Ball 1 / 5", (Vector2){screenWidth/2 - MeasureTextEx(font1,  "Ball 1 / 5", 40.0, 1.0).x/2 - 10,610}, 40, 1.0, WHITE);
                } else if (game.numLives == 4){
                    DrawTextEx(font1, "Ball 2 / 5", (Vector2){screenWidth/2 - MeasureTextEx(font1,  "Ball 2 / 5", 40.0, 1.0).x/2 - 10,610}, 40, 1.0, WHITE);
                } else if (game.numLives == 3){
                    DrawTextEx(font1, "Ball 3 / 5", (Vector2){screenWidth/2 - MeasureTextEx(font1,  "Ball 3 / 5", 40.0, 1.0).x/2 - 10,610}, 40, 1.0, WHITE);
                } else if (game.numLives == 2){
                    DrawTextEx(font1, "Ball 4 / 5", (Vector2){screenWidth/2 - MeasureTextEx(font1,  "Ball 4 / 5", 40.0, 1.0).x/2 - 10,610}, 40, 1.0, WHITE);
                } else if (game.numLives == 1){
                    DrawTextEx(font1, "Ball 5 / 5", (Vector2){screenWidth/2 - MeasureTextEx(font1,  "Ball 5 / 5", 40.0, 1.0).x/2 - 10,610}, 40, 1.0, WHITE);
                }
                DrawTextEx(font1, "Center Button to Launch!", (Vector2){screenWidth/2 - MeasureTextEx(font1,  "Center Button to Launch!", 20.0, 1.0).x/2  - 10,650}, 20, 1.0, WHITE);

                for (int i = 0; i < 8; i++){
                    DrawTexturePro(arrowRight,(Rectangle){0,0,arrowRight.width,arrowRight.height},(Rectangle){screenWidth - 9,(i * 20) + 625+ (5 * sin(((i*100)+millis()-elapsedTimeStart)/200.0f)),20,20},(Vector2){16,16},-90,(Color){0,0,0,100});
                }
            }

            // Debug Rendering
            if (IsKeyDown(KEY_TAB)){
                DrawFPS(10, 10);

                DrawLine(0,mouseY,screenWidth,mouseY,RED);
                DrawLine(mouseX,0,mouseX,screenHeight,RED);
                if (IsMouseButtonPressed(0)){
                    printf("{%f,%f,,},\n",(float)(mouseX * screenToWorld),(float)(mouseY * screenToWorld));
                }

                for (int i = 0; i < numWalls; i++){
                    DrawLineEx((Vector2){walls[i][0]*worldToScreen,walls[i][1]*worldToScreen},(Vector2){walls[i][2]*worldToScreen,walls[i][3]*worldToScreen},1,GREEN);
                    DrawCircle(walls[i][0]*worldToScreen,walls[i][1]*worldToScreen,2,RED);
                    DrawCircle(walls[i][2]*worldToScreen,walls[i][3]*worldToScreen,2,RED);
                }

                cpSpaceDebugDraw(space, &drawOptions);
            }
        }
        if (game.gameState == 2){
            ClearBackground((Color){255,183,0,255});
            float timeFactor = (millis() - elapsedTimeStart) / 1000.0f;
            float xOffset = sin(timeFactor) * 50.0f;
            float yOffset = cos(timeFactor) * 50.0f;
            float angle = sin(timeFactor * 2) * 20 + cos(timeFactor / 3) * 25;
            float width = screenWidth * 3;
            float height = screenHeight * 3;
            SetShaderValue(swirlShader, secondsLoc, &shaderSeconds, UNIFORM_FLOAT);
			BeginShaderMode(swirlShader);
            DrawTexturePro(bgMenu,(Rectangle){0,0,bgMenu.width,bgMenu.height},(Rectangle){xOffset + screenWidth/2,yOffset + screenWidth/2,width,height},(Vector2){width/2,height/2},angle,WHITE);
            EndShaderMode();

            for (int i = 0; i < 16; i++){
                DrawTexturePro(ballTex,(Rectangle){0,0,ballTex.width,ballTex.height},(Rectangle){menuPinballs[i].px,menuPinballs[i].py,30,30},(Vector2){0,0},0,(Color){0,0,0,50});
            }

            DrawTexturePro(gameOverOverlay1,(Rectangle){0,0,gameOverOverlay1.width,gameOverOverlay1.height},(Rectangle){0,0,screenWidth,screenHeight},(Vector2){0,0},0,WHITE);
            DrawTexturePro(gameOverOverlay2,(Rectangle){0,0,gameOverOverlay2.width,gameOverOverlay2.height},(Rectangle){0,12 + sin((millis() - elapsedTimeStart) / 1000.0f)*5.0f,screenWidth,screenHeight},(Vector2){0,0},0,WHITE);

            sprintf(tempString,"%ld",game.gameScore);
            DrawTextEx(font2, "Score:", (Vector2){screenWidth/2 - MeasureTextEx(font2, "Score:", 60, 1.0).x/2,275}, 60, 1.0, WHITE);
            DrawTextEx(font2, tempString, (Vector2){screenWidth/2 - MeasureTextEx(font2, tempString, 60, 1.0).x/2,332}, 60, 1.0, WHITE);

            for (int i =0; i < 5; i++){
                sprintf(tempString,"%c",nameString[i]);
                float textWidth = MeasureTextEx(font2, tempString, 60, 1.0).x;
                if (nameString[i] == 32){
                    DrawTextEx(font2, "-", (Vector2){54 + (i * 62) - textWidth / 2,510}, 60, 1.0, DARKGRAY);
                } else {
                    DrawTextEx(font2, tempString, (Vector2){54 + (i * 62) - textWidth / 2,510}, 60, 1.0, WHITE);
                }
            }
            DrawTexturePro(arrowRight,(Rectangle){0,0,arrowRight.width,arrowRight.height},(Rectangle){54 + (game.nameSelectIndex * 62),595+ (5 * sin((millis()-elapsedTimeStart)/200.0f)),32,32},(Vector2){16,16},-90,WHITE);

        }
        if (game.gameState == 5){
            ClearBackground(WHITE);
        }

        if (game.transitionState > 0){
            SetShaderValue(swirlShader, secondsLoc, &shaderSeconds, UNIFORM_FLOAT);
            float transitionAmount = ((game.transitionAlpha / 255.0f));
            DrawRectanglePro((Rectangle){screenWidth,screenHeight,screenWidth,screenHeight + 200}, (Vector2){0,screenHeight + 200}, -33.0f * transitionAmount, BLACK);
            DrawRectanglePro((Rectangle){0,0,screenWidth,screenHeight + 200}, (Vector2){screenWidth,0}, -33.0f * transitionAmount, BLACK);
        }

        EndDrawing();
    }

    shutdownScores(scores);
    inputShutdown(input);
    shutdownSound(sound);
    CloseWindow();

    return 0;
}
