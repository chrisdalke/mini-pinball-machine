#rm dashboard_mac
#export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/Cellar/raylib/2.0.0/lib/pkgconfig
clang -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL lib/raylib_mac/libraylib_mac.a lib/chipmunk_mac/libchipmunk_mac.a main.c physicsDebugDraw.c constants.c inputManagerMac.c soundManager.c scores.c gameStruct.c -o game_mac -L./lib/chipmunk_mac -I./lib/chipmunk_mac -I./lib/raylib_mac -L/opt/local/lib -lsqlite3 -lcurl -lssl -lcrypto -lssl -lcrypto -lz
./game_mac
