#ifndef HEADER_PHYSICS_DEBUG_DRAW
#define HEADER_PHYSICS_DEBUG_DRAW

void ChipmunkDebugDrawCircle(cpVect pos, cpFloat angle, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor);
void ChipmunkDebugDrawSegment(cpVect a, cpVect b, cpSpaceDebugColor color);
void ChipmunkDebugDrawFatSegment(cpVect a, cpVect b, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor);
void ChipmunkDebugDrawPolygon(int count, const cpVect *verts, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor);
void ChipmunkDebugDrawDot(cpFloat size, cpVect pos, cpSpaceDebugColor fillColor);
cpSpaceDebugColor ChipmunkDebugGetColorForShape(cpShape *shape, cpDataPointer data);

#endif
