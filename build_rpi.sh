gcc -o game_rpi main.c -O1 -s -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -std=gnu99 -I. -I/home/pi/raylib/src -I/home/pi/raylib/src/external -I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads -L. -L./lib -L/home/pi/raylib/src -L/opt/vc/lib -lraylib -lbrcmGLESv2 -lbrcmEGL -lpthread -lrt -lm -lbcm_host -ldl -DPLATFORM_RPI
./game_rpi
