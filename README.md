claytracer
==========

My experiments using OpenCL to build a ray tracer.

Dependencies: SDL2 http://www.libsdl.org/download-2.0.php

*OS X Compile*

    clang -o claytracer --std=c11 -framework OpenCL -framework SDL2 *.c

TODO List
=========

* Fix the warping of the image <s>near the edges of the screen</s> when objects are at high viewing angles and the aspect ratio isn't 1:1.
* Allow variable amount of lights in the scene.
* Implement functionality to allow the user to move the camera via user input to "fly around" the scene.
