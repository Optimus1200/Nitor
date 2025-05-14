# Nitor - OpenGL 3D Graphics Engine

![Image](https://raw.githubusercontent.com/Optimus1200/Images/main/nitor_preview.png)

## Features
* **Physically-Based Rendering (PBR)**
* **Directional Lighting**
* **Cascaded Shadow Mapping**
* **Texture Loading and Model Importing**
* **Entity Component System (ECS)**

## Camera Controls
* Scene must be in focus, click on an empty space outside of the GUI.
* Press WASD keys to move.
* Right mouse click and drag to rotate .

## Requirements
* **Language:** C++17
* **API:** OpenGL 4.6

## Build with CMake
**Windows**
```
mkdir build
cd build
cmake ..
cmake --build . -- /m:4
```
**Unix/Linux**
```
mkdir build
cd build
cmake ..
cmake --build . -j4
```

## Libraries
* **Assimp:** https://github.com/assimp/assimp
* **EnTT:** https://github.com/skypjack/entt
* **GLAD:** https://github.com/Dav1dde/glad
* **GLFW:** https://github.com/glfw/glfw
* **GLM:** https://github.com/g-truc/glm
* **ImGui:** https://github.com/ocornut/imgui
* **STB:** https://github.com/nothings/stb

## Resources
* **LearnOpenGL:** https://learnopengl.com
