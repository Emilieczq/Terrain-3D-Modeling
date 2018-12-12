# Terrain Modeling (OpenGl/GLUT)
This is a graphic program using C++ and OpenGL to produce and display an interactive terrain mesh.

![](terrain.gif)

## Features
There are two display windows: one is 3D Model, the other is 2D overview.
**Attention** To show sysnchronized 2D overview, you should select the small window (Windows) or press any key except q or esc (MacOS).

## Instruction
Under the path of makefile, use the command `make` to start the program.

| Commands | Functions |
|----------:|-----------|
| LEFT/RIGHT | rotate the 3D model about y axis |
| a/d | rotate the 3D model about x axis |
| i | print current rotation info |
| r | generate a new random terrain using current heightmap algorithm |
| f | toggle current algorithm between circle algorithm and fault algorithm, the default algorithm is circle algorithm |
| y | draw the terrain using quads (strips) |
| t | draw the terrain using triangles (strips) |
| w | toggle wireframe mode between three options: 1. solid polygons; 2. wireframe; 3. both solid polygons and wireframe |
| l | turn on/off lights |
| s | toggle current shading between flat shading and Gouraud shading, the default shading is Gouraud shading |
| q/esc | close the windows |

## Remarks
- Unfortunately, the 2D overview is not synchronized immediately with the 3D model.
