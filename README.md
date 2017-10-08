SpacePong
---------

The exciting space-based pong game written by Brandon Foltz as practice for coding in C.

### Words of Caution

I wrote this game years ago when I was not nearly as good at programming as I am now. As such, the source code serves as an example of how *not* to write a nice C program.
There are many examples of no-no's such as prolific global variables and mutable state everywhere, very very long source files, long functions, etc. 
It may be an interesting exercise to try and refactor the code in a better style. 

Compiling
---------

On Ubuntu 16.04, you can use the following series of commands to compile and play SpacePong.

```
sudo apt install libsdl-gfx1.2-5 libsdl-gfx1.2-dev libsdl-image1.2 libsdl-image1.2-dev libsdl-mixer1.2 libsdl-mixer1.2-dev libsdl-sound1.2 libsdl-sound1.2-dev libsdl-ttf2.0-0 libsdl-ttf2.0-dev libsdl1.2-dev libsdl1.2debian

./compile.sh

./SpacePong

```

Controls
--------

* Up/Down: controls Player 1.
* A/Z: controls Player 2 (if enabled).
* D: Toggle debug mode.
* F: Toggle fullscreen.
* B: Load new background image.
* P: Pause.
* M: Return to menu.
* Esc: Quit.

License
-------

This game was written by Brandon Foltz and is released under the Gnu GPL V3 license. See LICENSE.txt for details.
