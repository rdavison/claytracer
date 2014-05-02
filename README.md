claytracer
==========

My experiments using OpenCL to build a ray tracer.

*OS X Compile*

    clang -o claytracer --std=c11 -framework OpenCL -framework SDL2 *.c

TODO List
=========

* Allow points to generate more than one ray. For example, a ray hitting a mirror needs to generate a reflected ray AND a ray to seek a light, that way reflected surfaces can be subject to shadows.
