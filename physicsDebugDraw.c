#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <chipmunk.h>
#include "constants.h"
#include "physicsDebugDraw.h"

static inline cpSpaceDebugColor RGBAColor(float r, float g, float b, float a){
	cpSpaceDebugColor color = {r, g, b, a};
	return color;
}

static inline cpSpaceDebugColor LAColor(float l, float a){
	cpSpaceDebugColor color = {l, l, l, a};
	return color;
}

static inline Color ChipmunkToRaylibColor(cpSpaceDebugColor color){
    return (Color){(int)(color.r * 255.0f),(int)(color.g * 255.0f),(int)(color.b * 255.0f),(int)(color.a * 255.0f)};
}

cpSpaceDebugColor ChipmunkDebugGetColorForShape(cpShape *shape, cpDataPointer data){
	if(cpShapeGetSensor(shape)){
		return LAColor(1.0f, 0.1f);
	} else {
		cpBody *body = cpShapeGetBody(shape);

		if(cpBodyIsSleeping(body)){
			return RGBAColor(0x58/255.0f, 0x6e/255.0f, 0x75/255.0f, 1.0f);
		} else {
			return RGBAColor(0x93/255.0f, 0xa1/255.0f, 0xa1/255.0f, 1.0f);
		}
	}
}


void ChipmunkDebugDrawCircle(cpVect pos, cpFloat angle, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor){
    DrawCircle(pos.x * worldToScreen,pos.y* worldToScreen, radius* worldToScreen, ChipmunkToRaylibColor(fillColor));
    DrawCircleLines(pos.x* worldToScreen,pos.y* worldToScreen, radius* worldToScreen, ChipmunkToRaylibColor(outlineColor));
}

void ChipmunkDebugDrawSegment(cpVect a, cpVect b, cpSpaceDebugColor color){
    DrawLine(a.x* worldToScreen,a.y* worldToScreen,b.x* worldToScreen,b.y* worldToScreen,ChipmunkToRaylibColor(color));
}

void ChipmunkDebugDrawFatSegment(cpVect a, cpVect b, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor){
    DrawLineEx((Vector2){a.x* worldToScreen,a.y* worldToScreen},(Vector2){b.x* worldToScreen,b.y* worldToScreen},(radius* worldToScreen) / 2.0f,ChipmunkToRaylibColor(fillColor));
    DrawCircle(a.x* worldToScreen,a.y* worldToScreen,radius* worldToScreen,ChipmunkToRaylibColor(fillColor));
    DrawCircle(b.x* worldToScreen,b.y* worldToScreen,radius* worldToScreen,ChipmunkToRaylibColor(fillColor));
    DrawCircleLines(a.x* worldToScreen,a.y* worldToScreen,radius* worldToScreen,ChipmunkToRaylibColor(outlineColor));
    DrawCircleLines(b.x* worldToScreen,b.y* worldToScreen,radius* worldToScreen,ChipmunkToRaylibColor(outlineColor));
}

void ChipmunkDebugDrawPolygon(int count, const cpVect *verts, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor){
    //Vector2* points = malloc(sizeof(Vector2) * count);
    float prevPointX = 0;
    float prevPointY = 0;
    for (int i = 0; i < count; i++){
        //points[i].x = verts[i].x;
        //points[i].y = verts[i].y;
        if (i > 0){
            ChipmunkDebugDrawSegment(cpv(prevPointX,prevPointY),cpv(verts[i].x,verts[i].y),outlineColor);
        }
        prevPointX = verts[i].x;
        prevPointY = verts[i].y;
    }
    ChipmunkDebugDrawSegment(cpv(verts[count - 1].x,verts[count - 1].y),cpv(verts[0].x,verts[0].y),outlineColor);
    for (int i = 0; i < count; i++){
        ChipmunkDebugDrawDot(1.0f,verts[i],outlineColor);
    }
}

void ChipmunkDebugDrawDot(cpFloat size, cpVect pos, cpSpaceDebugColor fillColor){
    DrawCircle(pos.x* worldToScreen,pos.y* worldToScreen,size* worldToScreen / 2.0f,ChipmunkToRaylibColor(fillColor));
}
