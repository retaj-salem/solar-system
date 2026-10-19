<div align="center">

# 🪐 Interactive Solar System

**A 3D animated model of the Solar System built with C++ and OpenGL**

![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![OpenGL](https://img.shields.io/badge/OpenGL-5586A4?style=for-the-badge&logo=opengl&logoColor=white)
![Computer Graphics](https://img.shields.io/badge/Computer%20Graphics-1a1a2e?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Completed-success?style=for-the-badge)

<img src="images/main-scene.png" alt="Main solar system scene" width="85%">

</div>

---

## 📖 About the Project

Interactive Solar System is a group project developed at **Umm Al-Qura University**. It displays the **Sun and the eight planets**, each orbiting the Sun and rotating around its own axis. The user can click planets to learn about them, rotate the camera, zoom, and pause the animation.

The goal of the project is to apply core computer graphics concepts in a practical, visual way: 3D rendering, transformations, lighting, texture mapping, and user interaction.

## ✨ Features

- 🌞 3D Sun at the center with the 8 planets around it
- 🔄 Orbit animation and self-rotation for every planet
- 🛰️ Visible orbit paths and rings for Saturn and Uranus
- 💡 Lighting (ambient, diffuse, specular) placed at the Sun
- 🖼️ Texture mapping with BMP images, with an automatic colored fallback if a texture is missing
- 🌌 Dark space background with stars
- 🖱️ Click a planet to enlarge it and show its name and a short description
- 🎥 Camera rotation, zoom in/out, pause/resume, and reset view

## 🎮 Controls

| Input | Action |
|-------|--------|
| **Left mouse click** | Select a planet (enlarges it and shows its info) |
| **Space** | Pause / resume the animation |
| **Arrow keys** | Rotate the camera |
| **+ / -** | Zoom in / zoom out |
| **R** | Reset the view and clear the selection |
| **Esc** | Exit the application |

## 📸 Screenshots

| Planet selection | Zoom in | Zoom out |
|:---:|:---:|:---:|
| <img src="images/planet-selection.png" width="300"> | <img src="images/zoom-in.png" width="300"> | <img src="images/zoom-out.png" width="300"> |

## 🧰 Technologies

| Tool | Usage |
|------|-------|
| **C++** | Main programming language |
| **OpenGL** | Rendering the 3D scene, planets, orbits, and transformations |
| **GLU** | 3D objects and perspective projection |
| **GLUT / FreeGLUT** | Window creation and mouse/keyboard input |
| **GLEW** | OpenGL extension management (non-Apple systems) |
| **BMP textures** | Textures for the Sun and the planets |

## 🧠 How It Works

- **Rendering:** `display()`, `drawSun()`, `drawPlanet()`, `drawHUD()` draw each body as a 3D sphere plus on-screen text.
- **Animation:** `updateAnimation()` updates `orbitAngle` and `selfAngle` using each planet's orbit and rotation speed.
- **Lighting:** `initializeLighting()` sets up `GL_LIGHT0` near the Sun.
- **Textures:** `LoadTexture()` and `initializeTextures()` load the BMP files, and `createFallbackTexture()` creates a colored texture if a file is missing.
- **Transformations:** `glTranslatef`, `glRotatef`, `glScalef`, and `gluPerspective` for positioning, rotation, scaling, and perspective.
- **Interaction:** `mouse()` with `selectPlanetByMouse()`, `key()`, and `special()` handle selection, zoom, pause, reset, and camera rotation.

## 🚀 Getting Started

### Requirements
- A C++ compiler (Visual Studio, g++, or clang)
- OpenGL, GLU, FreeGLUT, and GLEW installed

### Project structure
```
solar-system/
├── MainPR.cpp
├── assets/          # sun.bmp, mercury.bmp, ... neptune.bmp
├── images/          # README screenshots
└── README.md
```

> The code loads textures from `../assets/`, so run the executable from a folder next to `assets/` (for example a `build/` folder). If textures are not found, the program still runs with colored planets.

### Build and run

**Linux**
```bash
g++ MainPR.cpp -o solar_system -lGLEW -lglut -lGLU -lGL
./solar_system
```

**macOS**
```bash
g++ MainPR.cpp -o solar_system -framework OpenGL -framework GLUT
./solar_system
```

**Windows (Visual Studio):** create a new C++ project, add `MainPR.cpp`, link FreeGLUT and GLEW, then build and run.

## 🔮 Future Work

- Add the Moon, asteroids, and comets
- Improve the accuracy of planet sizes, distances, and speeds
- Add more detailed information for each planet
- Add a menu system for easier control
- Camera control with mouse dragging
- Higher-quality textures, background sound, and visual effects

## 👩‍💻 My Contribution

This was a group project. I worked on the user interaction and controls: mouse planet selection, camera rotation, zoom, pause/resume, and reset.

---

<div align="center">

Made with ❤️ at Umm Al-Qura University

</div>
