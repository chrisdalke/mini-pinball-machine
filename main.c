#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <chipmunk.h>

#define DEG_TO_RAD (3.14159265 / 180.0)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

long long millis() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

typedef struct {
    int active;
    cpShape *shape;
    cpBody *body;
} Ball;

typedef struct {
    cpShape *shape;
    cpBody *body;
    float bounceEffect;
} Bumper;

typedef struct {
    cpSpace *space;
    int numBalls;
    Ball *balls;
    int active;
} GameStruct;

enum CollisionTypes {
    COLLISION_WALL = 0,
	COLLISION_BALL = 1,
    COLLISION_BUMPER = 2,
    COLLISION_PADDLE = 3
};

const int numWalls = 64;
const int maxBalls = 16;
const float ballSize = 5;

// Collision handlers
static cpBool CollisionHandlerBallBumper(cpArbiter *arb, cpSpace *space, void *ignore){
	CP_ARBITER_GET_SHAPES(arb, a, b);
	Bumper* bumper = (Bumper *)cpShapeGetUserData(b);
    Ball *ball = (Ball *)cpShapeGetUserData(a);

    // On the bumper object, set the collision effect
    bumper->bounceEffect = 10.0f;

    // On the ball object, add a random velocity
    cpBodyApplyImpulseAtLocalPoint(ball->body,cpv(rand() % 20 - 10, rand() % 20 - 10),cpvzero);

	return cpTrue;
}

// Add ball function
void addBall(GameStruct *game, float px, float py, float vx, float vy){
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

int main(void){
    const int screenWidth = 450;
    const int screenHeight = 800;
    const int worldWidth = 90;
    const int worldHeight = 160;

    const float worldToScreen = (float)screenWidth / (float)worldWidth;
    const float screenToWorld = (float)worldWidth / (float)screenWidth;

    // Initialize a struct encoding data about the game.
    GameStruct game;


    float walls[64][4] = {
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
    };
    writeCircleWallSegment(walls,11,20,0,90,worldWidth-28.5,30.75,28.75);
    writeCircleWallSegment(walls,31,20,270,360,28.5,30.75,28.75);

    InitAudioDevice();
    Sound sound = LoadSound("Resources/Audio/1.mp3");
    PlaySound(sound);

    //SetConfigFlags(FLAG_SHOW_LOGO | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Mini Pinball by Chris Dalke!");
    SetTargetFPS(60);

    Texture bgTex = LoadTexture("Resources/Textures/background2.png");
    Texture ballTex = LoadTexture("Resources/Textures/ball.png");
    Texture bumperTex = LoadTexture("Resources/Textures/bumper.png");
    Texture shockwaveTex = LoadTexture("Resources/Textures/shockwave.png");

    // Initialize physics simulation
    cpVect gravity = cpv(0,100);
    cpSpace *space = cpSpaceNew();
    game.space = space;
    cpSpaceSetGravity(space,gravity);
    cpFloat timeStep = 1.0/60.0;

    // create walls
    for (int i = 0; i < numWalls; i++){
        cpShape *wall = cpSegmentShapeNew(cpSpaceGetStaticBody(space),cpv(walls[i][0],walls[i][1]),cpv(walls[i][2],walls[i][3]),0);
        cpShapeSetFriction(wall,0);
        cpShapeSetElasticity(wall,0.7);
        cpShapeSetCollisionType(wall, COLLISION_WALL);
        cpSpaceAddShape(space,wall);
    }

    // Create bumpers
    const int numBumpers = 3;
    const float bumperSize = 10.0f;
    const float bumperBounciness = 2.0f;
    Bumper* bumpers = malloc(numBumpers * sizeof(Bumper));

    bumpers[0].body = cpSpaceAddBody(space,cpBodyNewKinematic());
    cpBodySetPosition(bumpers[0].body,cpv(20.4,20));
    bumpers[0].shape = cpSpaceAddShape(space,cpCircleShapeNew(bumpers[0].body,bumperSize/2.0f,cpvzero));
    cpShapeSetElasticity(bumpers[0].shape,bumperBounciness);
    cpShapeSetCollisionType(bumpers[0].shape, COLLISION_BUMPER);
    cpShapeSetUserData(bumpers[0].shape,&bumpers[0]);
    bumpers[0].bounceEffect = 0;

    bumpers[1].body = cpSpaceAddBody(space,cpBodyNewKinematic());
    cpBodySetPosition(bumpers[1].body,cpv(42.4,18.4));
    bumpers[1].shape = cpSpaceAddShape(space,cpCircleShapeNew(bumpers[1].body,bumperSize/2.0f,cpvzero));
    cpShapeSetElasticity(bumpers[1].shape,bumperBounciness);
    cpShapeSetCollisionType(bumpers[1].shape, COLLISION_BUMPER);
    cpShapeSetUserData(bumpers[1].shape,&bumpers[1]);
    bumpers[1].bounceEffect = 0;

    bumpers[2].body = cpSpaceAddBody(space,cpBodyNewKinematic());
    cpBodySetPosition(bumpers[2].body,cpv(33.6,36.8));
    bumpers[2].shape = cpSpaceAddShape(space,cpCircleShapeNew(bumpers[2].body,bumperSize/2.0f,cpvzero));
    cpShapeSetElasticity(bumpers[2].shape,bumperBounciness);
    cpShapeSetCollisionType(bumpers[2].shape, COLLISION_BUMPER);
    cpShapeSetUserData(bumpers[2].shape,&bumpers[2]);
    bumpers[2].bounceEffect = 0;

    //Add collision handler for ball-bumper effect
    cpCollisionHandler *handler = cpSpaceAddCollisionHandler(space,COLLISION_BALL,COLLISION_BUMPER);
    handler->beginFunc = CollisionHandlerBallBumper;


    //create balls array
    Ball* balls = malloc(maxBalls * sizeof(Ball));
    game.balls = balls;
    game.numBalls = 0;
    for (int i = 0; i < maxBalls; i++){
        balls[i].active = 0;
    }

    // Setup timestepping system
    int timestep = 1000.0/60.0;
    long long accumulatedTime = 0;
    long long startTime = millis();
    long long endTime = millis();

    while (!WindowShouldClose()){
        endTime = millis();
        accumulatedTime += (endTime - startTime);
        startTime = millis();

        // STEP SIMULATION AT FIXED RATE
        while (accumulatedTime > timestep){
            accumulatedTime -= timestep;
            cpSpaceStep(space, timeStep / 2.0);
            cpSpaceStep(space, timeStep / 2.0);

            if (IsKeyPressed(KEY_SPACE)){
                addBall(&game,89.5 - ballSize / 2,160,0,-200);
            }
            if (game.numBalls == 0){
                addBall(&game,89.5 - ballSize / 2,160,0,-200);
            }

            // Check if any balls have fallen outside the screen
            // Remove them if they have.
            for (int i = 0; i < maxBalls; i++){
                if (balls[i].active == 1){
                    cpVect pos = cpBodyGetPosition(balls[i].body);
                    if (pos.y > 170+ballSize){
                        balls[i].active = 0;
                        cpSpaceRemoveShape(game.space,balls[i].shape);
                        cpSpaceRemoveBody(game.space,balls[i].body);
                        cpShapeFree(balls[i].shape);
                        cpBodyFree(balls[i].body);
                        game.numBalls--;
                    }
                }
            }

            // Update bumper
            for (int i = 0; i < numBumpers; i++){
                bumpers[i].bounceEffect *= 0.94;
            }
        }

        // RENDER AT SPEED GOVERNED BY RAYLIB
        BeginDrawing();
        DrawTexturePro(bgTex,(Rectangle){0,0,bgTex.width,bgTex.height},(Rectangle){0,0,screenWidth,screenHeight},(Vector2){0,0},0,WHITE);

        // Render balls
        for (int i = 0; i < maxBalls; i++){
            if (balls[i].active == 1){
                cpVect pos = cpBodyGetPosition(balls[i].body);
                DrawTexturePro(ballTex,(Rectangle){0,0,ballTex.width,ballTex.height},(Rectangle){pos.x * worldToScreen,pos.y * worldToScreen,ballSize * worldToScreen,ballSize * worldToScreen},(Vector2){(ballSize / 2.0) * worldToScreen,(ballSize / 2.0) * worldToScreen},0,WHITE);
            }
        }

        // Render bumpers
        for (int i = 0; i < numBumpers; i++){
            cpVect pos = cpBodyGetPosition(bumpers[i].body);
            float bounceScale = 0.2f;
            float width = bumperSize + cos(millis() / 20.0) * bumpers[i].bounceEffect * bounceScale;
            float height = bumperSize + sin(millis() / 20.0) * bumpers[i].bounceEffect * bounceScale;
            float shockSize = (bumperSize * bumpers[i].bounceEffect) * 0.15f;
            DrawTexturePro(shockwaveTex,(Rectangle){0,0,shockwaveTex.width,shockwaveTex.height},(Rectangle){pos.x * worldToScreen,pos.y * worldToScreen,shockSize * worldToScreen,shockSize * worldToScreen},(Vector2){shockSize/2 * worldToScreen,shockSize/2 * worldToScreen},0,WHITE);
            DrawTexturePro(bumperTex,(Rectangle){0,0,bumperTex.width,bumperTex.height},(Rectangle){pos.x * worldToScreen,pos.y * worldToScreen,width * worldToScreen,height * worldToScreen},(Vector2){(width / 2.0) * worldToScreen,(height / 2.0) * worldToScreen},0,WHITE);
        }

        // DEBUG RENDERING
        if (IsKeyDown(KEY_TAB)){
            DrawFPS(10, 10);
            DrawText("MINI PINBALL BY CHRIS DALKE", 10, 40, 20, GRAY);

            float mouseX = GetMouseX() * 2.0;
            float mouseY = GetMouseY() * 2.0;
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
        }

        EndDrawing();
    }

    UnloadSound(sound);
    CloseAudioDevice();

    CloseWindow();

    return 0;
}
