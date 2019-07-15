from raylibpy import *
import pymunk
import math
from time import time

class GameObject():
    def __init__(self,texture):
        self.position = Vector2(0,0)
        self.size = Vector2(0,0)
        self.origin = Vector2(0,0)
        self.angle = 0.0
        self.texture = texture
        self.dead = False
        self.parent = None
    def update(self):
        pass
    def renderBg(self):
        pass
    def renderOverlay(self):
        pass
    def render(self):
        draw_texture_pro(self.texture, Rectangle(0,0,self.texture.width,self.texture.height), Rectangle(self.position.x,self.position.y,self.size.x,self.size.y), self.origin, 0, WHITE)
    def addPhysics(self,world):
        pass
    def removePhysics(self,world):
        pass

class Ball(GameObject):
    def __init__(self,texture):
        super().__init__(texture)
        self.mass = 1
        self.diameter = 20
        self.inertia = pymunk.moment_for_circle(self.mass, 0, self.diameter / 2, (0,0))
        self.body = pymunk.Body(self.mass, self.inertia)
        self.poly = pymunk.Circle(self.body, self.diameter / 2, (0,0))
        self.poly.elasticity = 0.95
        self.poly.elasticity = 1
        self.poly.collision_type = 1
        self.size.x = self.diameter
        self.size.y = self.diameter
        self.origin.x = self.size.x / 2
        self.origin.y = self.size.y / 2
        self.poly.gameobject = self
    def update(self):
        super().update()
        self.position.x = self.body.position.x
        self.position.y = self.body.position.y
        if (self.position.y > 800):
            self.dead = True
    def render(self):
        super().render()
    def applyImpulse(self,x,y):
        self.body.apply_impulse_at_local_point((x,y))
    def addPhysics(self,world):
        self.body.position = self.position.x, self.position.y
        world.add(self.body,self.poly)
        pass
    def removePhysics(self,world):
        world.remove(self.body,self.poly)
        pass

class Bumper(GameObject):
    def __init__(self,texture):
        super().__init__(texture)
        self.diameter = 50
        self.size.x = self.diameter
        self.size.y = self.diameter
        self.origin.x = self.size.x / 2
        self.origin.y = self.size.y / 2
        self.bounceEffect = 0

        self.body = pymunk.Body(body_type=pymunk.Body.KINEMATIC)
        self.poly = pymunk.Circle(self.body, self.diameter / 2)
        self.poly.elasticity = 1.5
        self.poly.collision_type = 2
        self.poly.gameobject = self
    def update(self):
        super().update()
        self.bounceEffect *= 0.94
    def render(self):
        #super().render()
        bounceScale = 1
        width = self.size.x + math.cos(time() * 20.0) * self.bounceEffect * bounceScale
        height = self.size.y + math.sin(time() * 20.0) * self.bounceEffect * bounceScale
        draw_texture_pro(self.texture, Rectangle(0,0,self.texture.width,self.texture.height), Rectangle(self.position.x,self.position.y,width,height), Vector2(width/2,height/2), 0, WHITE)
    def addPhysics(self,world):
        self.body.position = self.position.x, self.position.y
        world.add(self.body,self.poly)
        pass
    def removePhysics(self,world):
        pass
    def bouncePinball(self,ball):
        self.bounceEffect = 10

def LeftBumper(GameObject):
    def __init__(self,texture):
        super().__init__(texture)
    def update(self):
        super().update()
    def render(self):
        super().render()
    def addPhysics(self,world):
        pass
    def removePhysics(self,world):
        pass

class ShockRing(GameObject):
    def __init__(self,texture):
        super().__init__(texture)
        self.scale = 0
    def update(self):
        self.scale += 0.05
        if (self.scale > 2.0):
            self.dead = True
        super().update()
    def render(self):
        self.width = 20.0 * self.scale
        self.height = 20.0 * self.scale
        self.origin.x = self.width / 2
        self.origin.y = self.width / 2
        super().render()
