gcc -o game_rpi main.c physicsDebugDraw.c constants.c inputManagerPi.c scores.c soundManager.c gameStruct.c sqlite3.c -O1 -s -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -std=gnu99 -I. -I/home/pi/raylib/src -I/home/pi/raylib/src/external -I/home/pi/Chipmunk-7.0.3/include/chipmunk -I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads -I/usr/local/include  -L. -L./lib -L/home/pi/raylib/src -L/home/pi/Chipmunk-7.0.3/src -L/opt/vc/lib -L/usr/local/lib -lwiringPi -lraylib -l:libchipmunk.a -lbrcmGLESv2 -lbrcmEGL -lpthread -lrt -lm -lbcm_host -ldl -DPLATFORM_RPI
./game_rpi
