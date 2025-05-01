# 🚗 NeedForSpeedInfinity

A fast-paced C++ racing game, built with OpenGL and GLUT, featuring dynamic obstacle avoidance and high-speed gameplay. Inspired by classics like *Need for Speed*, with modern mechanics and challenging gameplay.

---

## 🎮 About the Project

**NeedForSpeedInfinity** is a 3D/2D car game that combines high-speed racing with obstacle avoidance. The game features:
- Dynamic road scrolling with synchronized textures and lane markers
- Challenging obstacle patterns with increasing difficulty
- Enemy cars that move at double speed
- High score system with persistent storage
- Progressive difficulty scaling

### 🔧 Tech Stack

- **C++17**
- **OpenGL** with **FreeGLUT** and **GLEW** ([freeglut-MSVC-3.0.0-2.mp](https://www.transmissionzero.co.uk/files/software/development/GLUT/freeglut-MSVC.zip) | [glew-2.2.0-win32](https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip/download))
- **Visual Studio 2022**
- **Windows 10/11**

---

## 🚀 Getting Started

### 🛠 Prerequisites

- Visual Studio with **Desktop development with C++**
- OpenGL libraries:
  - `freeglut.dll`
  - `glew32.dll` (already included)

### 🔧 Build Instructions

1. Open `NeedForSpeedInfinity.sln` in Visual Studio.
2. Ensure build mode is set to `x64` and `Debug`.
3. Press `Ctrl + F5` to build and run.

---

## 📁 Project Structure

```bash
.
├── src/                    # Source files
│   ├── Car.cpp            # Car movement and physics
│   ├── Level.cpp          # Game level and obstacles
│   └── Texture.cpp        # Texture loading and management
├── include/               # Header files
├── assets/               # Game assets
├── lib/                  # External libraries
├── textures/            # Game textures
├── NeedForSpeedInfinity.sln      # Visual Studio solution
└── *.vcxproj / .filters / .user  # Project settings
```

---

## 🎯 Features & Gameplay

- **Dynamic Road System**
  - Smooth scrolling road with synchronized textures
  - White dashed center lines
  - Yellow border lines for road boundaries
  - Progressive speed increase

- **Obstacle System**
  - Multiple obstacle types (barriers, cones, rocks, trees)
  - Dynamic obstacle spawning with increasing frequency
  - Enemy cars that move at double speed
  - Collision detection and health system

- **Scoring & Progression**
  - High score system with persistent storage
  - Progressive difficulty levels
  - Score multiplier based on speed
  - Level-up system

---

## 💡 Recent Updates

- Implemented synchronized road textures and lane markers
- Added dynamic obstacle spawning with immediate start
- Introduced enemy cars with double-speed movement
- Improved game difficulty progression
- Added high score persistence

---

## 📜 License

This project is open-source under the MIT License.

---

## 👤 Author

**Prabesh Aryal**  
Computer Engineering Student, Baku Engineering University  
[GitHub](https://github.com/prabesharyal) • [LinkedIn](https://linkedin.com/in/prabesharyalnp)

