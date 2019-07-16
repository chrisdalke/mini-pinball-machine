#rm dashboard_mac
#export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/Cellar/raylib/2.0.0/lib/pkgconfig
clang -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL lib/libraylib_mac.a main.c -o game_mac -L/opt/local/lib -lcurl -lssl -lcrypto -lssl -lcrypto -lz
./game_mac
