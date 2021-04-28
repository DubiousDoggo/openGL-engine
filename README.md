# openGL-engine
A simple object viewer.  
I made this project mainly to learn openGL, and potentially turn it into a proper game engine at some point.
If you're interested in learning openGL check out Joey de Vries awesome book at [learnopengl.com](https://learnopengl.com).

## Usage
On launch it will ask for a model to load. Currently it only supports OBJ files. MTL files do work, however filenames cannot contain spaces. By default it loads the included peach's castle model from mario 64 (nintendo please don't sue me). Use the WASD keys to move and the arrow keys to look around. 

## Building
The project uses GLFW, GLAD, GLM, and stb image. The required files should be included, although they have only been tested on windows 10. opengl32 is platform specific and should be included with your OS.

### VSCode
The included task file should include everything required to build and compile. Run the `g++.exe build project` task and you should get a working executable.
### Manual
1. Shaders - Run the command below to generate the shaders header file. This takes any GLSL files in the shaders directoy and puts them in `shaders.h`.
```
py ./compile_shaders.py ./shaders/
``` 
2. Compile - Run the command below to compile the project.
```
g++ -std=c++17 ./glad.c ./main.cc -o ./main.exe -Iinclude -Llib -lglfw3 -lgdi32 -lopengl32
```
