#!/bin/bash
#gcc ./examples/$1.c -Wall -Wextra -o ./examples/$1 -lm 
gcc cadigo1_test.c -o cadigo_test -lm `sdl2-config --cflags --libs` -lSDL2_gfx
