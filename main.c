#include "raylib.h"
#include <stdio.h>
#include <sys/time.h>
#include <math.h>

#define DEG_TO_RAD (3.14159265 / 180.0)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

long long millis() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

int main(void){
    const int screenWidth = 450;
    const int screenHeight = 800;
    const int worldWidth = 90;
    const int worldHeight = 160;

    const float worldToScreen = (float)screenWidth / (float)worldWidth;

    int numWalls = 1;
    float walls[1][4] = {{0,0,100,100}};

    InitAudioDevice();
    Sound sound = LoadSound("Resources/Audio/1.mp3");
    PlaySound(sound);

    //SetConfigFlags(FLAG_SHOW_LOGO | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Mini Pinball by Chris Dalke!");
    SetTargetFPS(60);

    Texture bgTex = LoadTexture("Resources/Textures/background2.png");


    while (!WindowShouldClose()){
        BeginDrawing();
        DrawTexturePro(bgTex,(Rectangle){0,0,bgTex.width,bgTex.height},(Rectangle){0,0,screenWidth,screenHeight},(Vector2){0,0},0,WHITE);
        DrawFPS(10, 10);
        DrawText("MINI PINBALL BY CHRIS DALKE", 10, 40, 20, GRAY);
        DrawLine(0,0,0,screenHeight,BLUE);
        DrawLine(screenWidth,0,screenWidth,screenHeight,BLUE);


        for (int i = 0; i < 1; i++){
            DrawLineEx((Vector2){walls[i][0]*worldToScreen,walls[i][1]*worldToScreen},(Vector2){walls[i][2]*worldToScreen,walls[i][3]*worldToScreen},4,GREEN);
        }


        DrawCircle(screenWidth/2 + sin(millis() / 500.0) * 100,screenHeight/2 + cos(millis() / 500.0) * 100,20,RED);
        EndDrawing();
    }

    UnloadSound(sound);
    CloseAudioDevice();

    CloseWindow();

    return 0;
}
