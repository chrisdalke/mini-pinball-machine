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
    const int screenWidth = 480;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "Mini Pinball by Chris Dalke!");
    SetTargetFPS(60);

    while (!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawFPS(10, 10);
        DrawText("MINI PINBALL BY CHRIS DALKE", 10, 40, 20, GRAY);
        DrawCircle(screenWidth/2 + sin(millis() / 500.0) * 100,screenHeight/2 + cos(millis() / 500.0) * 100,20,RED);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
