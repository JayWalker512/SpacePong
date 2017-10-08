#!/bin/sh

gcc main.c Sound.c -o SpacePong -lSDL -lSDL_gfx -lSDL_ttf -lSDL_image -lSDL_mixer -I/usr/include/SDL
