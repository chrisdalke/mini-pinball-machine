#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <chipmunk.h>
#include "constants.h"
#include "physicsDebugDraw.h"
#include "inputManager.h"

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
    int gameState;
    int transitionState;
    float transitionAlpha;
    int numLives;
    int menuState;
} GameStruct;

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
    COLLISION_PADDLE = 3
};

const int numWalls = 64;
const int maxBalls = 256;
const float ballSize = 5;

// Collision handlers
static cpBool CollisionHandlerBallBumper(cpArbiter *arb, cpSpace *space, void *ignore){
	CP_ARBITER_GET_SHAPES(arb, a, b);
	Bumper* bumper = (Bumper *)cpShapeGetUserData(b);
    Ball *ball = (Ball *)cpShapeGetUserData(a);

    // On the bumper object, set the collision effect
    bumper->bounceEffect = 10.0f;

    // On the ball object, add a random velocity
    //cpBodyApplyImpulseAtLocalPoint(ball->body,cpv(rand() % 20 - 10, rand() % 20 - 10),cpvzero);

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
        float mass = 10.0;
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


int main(void){

    // Initialize a struct encoding data about the game.
    GameStruct game;
    game.gameState = 0;


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
    Texture debugTex = LoadTexture("Resources/Textures/debugSmall.png");
    Texture leftFlipperTex = LoadTexture("Resources/Textures/flipperL.png");
    Texture rightFlipperTex = LoadTexture("Resources/Textures/flipperR.png");
    Texture bgMenu = LoadTexture("Resources/Textures/bgMenu.png");
    Texture titleOverlay = LoadTexture("Resources/Textures/titleOverlay.png");
    Texture menuOverlay1 = LoadTexture("Resources/Textures/menuOverlay1.png");
    Texture arrowRight = LoadTexture("Resources/Textures/arrowRight.png");
    Texture menuControls = LoadTexture("Resources/Textures/menuControls.png");

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
    const int numBumpers = 3;
    const float bumperSize = 10.0f;
    const float bumperBounciness = 2.2f;
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
    cpShapeSetFriction(leftFlipperShape,0.7);
    cpShapeSetFriction(rightFlipperShape,0.7);
    cpShapeSetElasticity(leftFlipperShape,0.6);
    cpShapeSetElasticity(rightFlipperShape,0.6);
    cpBodySetCenterOfGravity(leftFlipperBody,cpv(flipperHeight/2.0f,flipperHeight/2.0f));
    cpBodySetCenterOfGravity(rightFlipperBody,cpv(flipperHeight/2.0f,flipperHeight/2.0f));
    float flipperSpeed = 800.0f;
    float leftFlipperAngle = 33.0f;
    float rightFlipperAngle = 147.0f;
    float flipperSpeedScalar = 1.0f;

    //create balls array
    Ball* balls = malloc(maxBalls * sizeof(Ball));
    game.balls = balls;
    game.numBalls = 0;
    for (int i = 0; i < maxBalls; i++){
        balls[i].active = 0;
    }

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

    // Setup timestepping system
    int timestep = 1000.0/60.0;
    long long accumulatedTime = 0;
    long long startTime = millis();
    long long endTime = millis();
    long long elapsedTimeStart = millis();

    //game.transitionState = 2;
    //game.transitionAlpha = 255;
    game.transitionState = 0;
    game.transitionAlpha = 0;

    game.menuState = 0;


    while (!WindowShouldClose()){
        endTime = millis();
        accumulatedTime += (endTime - startTime);
        startTime = millis();
        shaderSeconds += GetFrameTime() / 2.0f;


        float mouseX = GetMouseX() * 2.0;
        float mouseY = GetMouseY() * 2.0;

        // Poll input
        inputUpdate(input);

        // STEP SIMULATION AT FIXED RATE
        while (accumulatedTime > timestep){
            accumulatedTime -= timestep;
            float slowMotionFactor = 1.0f;
            float effectiveTimestep = (timeStep) * slowMotionFactor;

            if (game.transitionState == 1){
                game.transitionAlpha += 2;
                if (game.transitionAlpha > 255){
                    game.transitionState = 2;
                    game.transitionAlpha = 255;
                }
            } else if (game.transitionState == 2){
                game.transitionAlpha -= 2;
                if (game.transitionAlpha < 0){
                    game.transitionState = 0;
                    game.transitionAlpha = 0;
                }
            } else {
                game.transitionAlpha = 0;
            }

            if (game.gameState == 0){
                // Menu

                // Update pinballs
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

                if (inputCenterPressed(input)){
                    game.gameState = 1;
                    game.numLives = 3;
                }

                if (inputLeftPressed(input)){
                    game.menuState = 1;
                }
                if (inputRightPressed(input)){
                    game.menuState = 0;
                }
            }
            if (game.gameState == 1){
                // Game
                cpSpaceStep(space, effectiveTimestep / 2.0f);
                cpSpaceStep(space, effectiveTimestep / 2.0f);

                if (inputCenter(input)){
                    //addBall(&game,89.5 - ballSize / 2,160,0,-220);
                }
                if (game.numBalls == 0){
                    if (game.numLives > 0){
                        addBall(&game,89.5 - ballSize / 2,160,0,-220);
                        game.numLives -= 1;
                    } else {
                        // game over condition
                        game.gameState = 0;
                    }
                }

                //if (IsMouseButtonPressed(0)){
                //    addBall(&game,mouseX * screenToWorld,mouseY * screenToWorld,0,0);
                //}

                float oldAngleLeft = leftFlipperAngle;
                float oldAngleRight = rightFlipperAngle;
                float targetAngleLeft = 0.0f;
                float targetAngleRight = 0.0f;
                if (inputLeft(input)){
                    targetAngleLeft = -33.0f;
                    leftFlipperAngle -= (flipperSpeed * effectiveTimestep);
                    if (leftFlipperAngle < targetAngleLeft){
                        leftFlipperAngle = targetAngleLeft;
                    }
                } else {
                    targetAngleLeft = 33.0f;
                    leftFlipperAngle += (flipperSpeed * effectiveTimestep);
                    if (leftFlipperAngle > targetAngleLeft){
                        leftFlipperAngle = targetAngleLeft;
                    }
                }
                if (inputRight(input)){
                    targetAngleRight = 213.0f;
                    rightFlipperAngle += (flipperSpeed * effectiveTimestep);
                    if (rightFlipperAngle > targetAngleRight){
                        rightFlipperAngle = targetAngleRight;
                    }
                } else {
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
            SetShaderValue(swirlShader, secondsLoc, &shaderSeconds, UNIFORM_FLOAT);
			BeginShaderMode(swirlShader);
            DrawTexturePro(bgMenu,(Rectangle){0,0,bgMenu.width,bgMenu.height},(Rectangle){xOffset + screenWidth/2,yOffset + screenWidth/2,width,height},(Vector2){width/2,height/2},angle,WHITE);
			EndShaderMode();

            // Render pinballs
            for (int i = 0; i < 16; i++){
                DrawTexturePro(ballTex,(Rectangle){0,0,ballTex.width,ballTex.height},(Rectangle){menuPinballs[i].px,menuPinballs[i].py,30,30},(Vector2){0,0},0,WHITE);
            }

            DrawTexturePro(menuOverlay1,(Rectangle){0,0,titleOverlay.width,titleOverlay.height},(Rectangle){0,0,screenWidth,screenHeight},(Vector2){0,0},0,WHITE);
            DrawTexturePro(titleOverlay,(Rectangle){0,0,titleOverlay.width,titleOverlay.height},(Rectangle){0,12 + sin(timeFactor)*5.0f,screenWidth,screenHeight},(Vector2){0,0},0,WHITE);
            //DrawTexturePro(arrowRight,(Rectangle){0,0,arrowRight.width,arrowRight.height},(Rectangle){16,600,32,32},(Vector2){16,16},180,WHITE);
            //DrawTexturePro(arrowRight,(Rectangle){0,0,arrowRight.width,arrowRight.height},(Rectangle){screenWidth-16,600,32,32},(Vector2){16,16},0,WHITE);
            //DrawTexturePro(arrowRight,(Rectangle){0,0,arrowRight.width,arrowRight.height},(Rectangle){screenWidth/2,screenHeight-16,32,32},(Vector2){16,16},90,WHITE);
            if (IsMouseButtonPressed(0)){
                printf("{%f,%f}\n",(float)(mouseX),(float)(mouseY));
            }

            if (game.menuState == 0){
            } else if (game.menuState == 1){
                DrawTexturePro(menuControls,(Rectangle){0,0,menuControls.width,menuControls.height},(Rectangle){26,320,menuControls.width/2,menuControls.height/2},(Vector2){0,0},0,WHITE);

            }

        }
        if (game.gameState == 1){
            // Game
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

            // Render left flipper
            cpVect pos = cpBodyGetPosition(leftFlipperBody);
            cpFloat angle = cpBodyGetAngle(leftFlipperBody);
            DrawTexturePro(leftFlipperTex,(Rectangle){0,0,leftFlipperTex.width,leftFlipperTex.height},(Rectangle){pos.x * worldToScreen,pos.y * worldToScreen,flipperWidth * worldToScreen,flipperHeight * worldToScreen},(Vector2){0 * worldToScreen,0 * worldToScreen},(angle * RAD_TO_DEG),WHITE);
            // Render right flipper
            pos = cpBodyGetPosition(rightFlipperBody);
            angle = cpBodyGetAngle(rightFlipperBody);
            DrawTexturePro(rightFlipperTex,(Rectangle){0,0,rightFlipperTex.width,rightFlipperTex.height},(Rectangle){pos.x * worldToScreen,pos.y * worldToScreen,flipperWidth * worldToScreen,flipperHeight * worldToScreen},(Vector2){0 * worldToScreen,0 * worldToScreen},(angle * RAD_TO_DEG),WHITE);


            // DEBUG RENDERING
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

        if (game.transitionState > 0){
            DrawRectangle(0,0,screenWidth,screenHeight,(Color){255,255,255,game.transitionAlpha});
        }

        EndDrawing();
    }

    inputShutdown(input);
    UnloadSound(sound);
    CloseAudioDevice();

    CloseWindow();

    return 0;
}
