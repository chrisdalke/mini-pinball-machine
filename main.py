from raylibpy import *
import os
from gameobjects import *
from time import time
import math

def millis():
    return int(round(time() * 1000))

width = 480
height = 800

dir_path = os.path.dirname(os.path.realpath(__file__))
init_window(width,height, "Pinball Game")
set_target_fps(60)
os.chdir(dir_path)

# Setup colors
bgColor = get_color(0x38003B00)
textColor = get_color(0xFFB70000)
overlayColor = get_color(0x00000088)

# Load textures
bgTex = load_texture("Resources/Textures/background2.png")
shockringTex = load_texture("Resources/Textures/shockwave.png")
ballTex = load_texture("Resources/Textures/ball.png")
bumperTex = load_texture("Resources/Textures/bumper.png")
flipperLTex = load_texture("Resources/Textures/flipperL.png")
flipperRTex = load_texture("Resources/Textures/flipperR.png")
flipperTrailTex = load_texture("Resources/Textures/flipperTrail.png")
trailTex = load_texture("Resources/Textures/trail.png")
pixelTex = load_texture("Resources/Textures/pixel.png")

def draw_line_tex(start,end,width,color):
    angle = math.degrees(math.atan2(end.y-start.y,end.x-start.x))
    dist = math.hypot(end.x - start.x, end.y - start.y)
    draw_texture_pro(pixelTex, Rectangle(0,0,1,1), Rectangle(start.x,start.y,dist,width), Vector2(0,0), angle, color)


# Game state stuff
objects = []

# Setup physics world
world = pymunk.Space()
world.gravity = 0,20

walls =  [pymunk.Segment(world.static_body, (0,0), (480,0), 1.0)
         ,pymunk.Segment(world.static_body, (0,0), (0,800), 1.0)
         ,pymunk.Segment(world.static_body, (480,0), (480,800), 1.0)
         ,pymunk.Segment(world.static_body, (456,800), (456,280), 1.0)
         ,pymunk.Segment(world.static_body, (452,800), (452,280), 1.0)
         ,pymunk.Segment(world.static_body, (452,280), (456,280), 1.0)
         ,pymunk.Segment(world.static_body, (452,635), (353,708), 1.0)
         ,pymunk.Segment(world.static_body, (0,635), (100,708), 1.0)
         ,pymunk.Segment(world.static_body, (480,135), (348,10), 1.0)
         ]
for line in walls:
    line.elasticity = 0.7
    line.group = 1
world.add(walls)

bumper1 = Bumper(bumperTex)
bumper1.position.x,bumper1.position.y = (173,183)
objects.append(bumper1)
bumper1.addPhysics(world)
bumper2 = Bumper(bumperTex)
bumper2.position.x,bumper2.position.y = (106,98)
objects.append(bumper2)
bumper2.addPhysics(world)
bumper3 = Bumper(bumperTex)
bumper3.position.x,bumper3.position.y = (217,91)
objects.append(bumper3)
bumper3.addPhysics(world)

# ball - bumper collision handler
def ball_bumper_collision(arbiter, space, data):
    ball_shape = arbiter.shapes[0]
    bumper_shape = arbiter.shapes[1]
    bumper_shape.gameobject.bouncePinball(ball_shape.gameobject)
    return True
h = world.add_collision_handler(1,2)
h.begin = ball_bumper_collision

TICK_RATE = 1000.0/60.0
intervalStartTime = millis()
intervalEndTime = millis()
intervalDeltaTime = 0
accumulatedTime = 0

while not window_should_close():
    # ---------------------------------------------
    # Update game logic, if the timer says so
    intervalEndTime = millis()
    intervalDeltaTime = intervalEndTime - intervalStartTime
    accumulatedTime += intervalDeltaTime
    intervalStartTime = millis()

    while (accumulatedTime > TICK_RATE):
        accumulatedTime -= TICK_RATE

        # Update physics
        world.step(0.05)

        # Trim dead objects
        for object in objects:
            if object.dead:
                object.removePhysics(world)
        objects[:] = [x for x in objects if not x.dead]
        # Update objects
        for object in objects:
            object.update()

        if (is_key_pressed(KEY_SPACE)):
            ball = Ball(ballTex)
            ball.position.x = 467.5
            ball.position.y = 780
            objects.append(ball)
            ball.addPhysics(world)
            ball.applyImpulse(0,-250)

    # ---------------------------------------------
    # Render game

    begin_drawing()
    clear_background(bgColor)
    draw_texture_pro(bgTex, Rectangle(0,0,bgTex.width,bgTex.height), Rectangle(0,0,480,800), Vector2(0,0), 0, WHITE)

    # Render objects list
    for object in objects:
        object.renderBg()
    for object in objects:
        object.render()
    for object in objects:
        object.renderOverlay()

    for line in walls:
        draw_line_tex(Vector2(line.a.x,line.a.y),Vector2(line.b.x,line.b.y),1,GREEN)

    if is_mouse_button_pressed(0):
        shock = ShockRing(shockringTex)


    # Draw debug info if tab key is pressed
    if (is_key_down(KEY_TAB)):
        draw_line_tex(Vector2(get_mouse_x(),0),Vector2(get_mouse_x(),800),1,RED)
        draw_line_tex(Vector2(0,get_mouse_y()),Vector2(480,get_mouse_y()),1,RED)
        draw_rectangle_rec(Rectangle(0,0,width,200),overlayColor)
        draw_fps(10,10)
        draw_text(str(len(objects))+ " objects",10,40,20,WHITE)
        draw_text("mouse x = "+str(get_mouse_x()),10,60,20,WHITE)
        draw_text("mouse y = "+str(get_mouse_y()),10,80,20,WHITE)
    end_drawing()


close_window()
