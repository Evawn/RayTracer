# RayTracer
3D Scene Renderer by RayTracing using C++, OpenGL, and Intel Embree.

## INSTRUCTIONS TO RUN:
First, unzip ```ext.zip```
Then, in the main folder, run the following commands in the terminal:

```
mkdir build
cd build
cmake ..
make -j 8
```

This creates the executable RTRef. You can render a scene by typing:

```
./RTRef bunnyscene.dae
```
or 
```
./RTRef staircase.dae
```
or 
```
./RTRef hero.dae
```
