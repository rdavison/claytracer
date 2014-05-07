claytracer
==========

My experiments using OpenCL to build a ray tracer.

* Dependencies: SDL2 http://www.libsdl.org/download-2.0.php
* YouTube demo (as of 2014.5.7): http://youtu.be/tiJtqLTerPE

*OS X Compile*

    clang -o claytracer --std=c11 -framework OpenCL -framework SDL2 *.c

TODO List
=========

* Massive code refactor / cleanup required soon to clean out the spaghetti code!!!
* Figure out why SDL2 sometimes does not register events while rendering. I need to investigate Multithreading the gui.
* Fix the warping of the image <s>near the edges of the screen</s> when objects are at high viewing angles and the aspect ratio isn't 1:1.
* Allow variable amount of lights in the scene.
* Implement functionality to allow the user to move the camera via user input to "fly around" the scene.
* Figure out why the floor (and maybe the ceiling too) is moving down/up in the demo even though it should only be moving in the Z direction.
* Implement rotations.
* Implement matrices.
