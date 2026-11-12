# 🦕 Chrono Run: DN Dash
**An Interactive 2D Endless Runner with Real-Time Day-Night Cycle**

> **Course**: Computer Graphics & Image Processing Lab (CSE-402)  
> **Institution**: Metropolitan University, Sylhet — Dept. of CSE, Section B, 58th Batch  
> **Lecturer**: Shrabanti Chowdhury

---

## 👥 Team

| Name | Student ID | Responsibility |
| :--- | :--- | :--- |
| Nahdiul Islam Rony | 231-115-069 | Rendering, day-night cycle, parallax, sprite animation |
| Abu Ubayda | 231-115-080 | Physics, collision detection, scoring, HUD, game states |

---

## 📁 Project Structure

```
CG-IP-Project/
│
├── src/
│   └── main.cpp                  ← Complete C++ OpenGL source code
│
├── include/GL/                   ← FreeGLUT headers (freeglut.h, glut.h …)
├── lib/                          ← FreeGLUT static library (libfreeglut.a)
│
├── docs/
│   └── index.html                ← Web / browser version (GitHub Pages)
│
├── presentation/
│   └── Chrono_Run_DN_Dash_Presentation.pptx
│
├── .vscode/                      ← VS Code build / debug / IntelliSense config
│   ├── c_cpp_properties.json
│   ├── launch.json
│   ├── settings.json
│   └── tasks.json
│
├── ChronoRun.exe                 ← Pre-compiled Windows executable
├── ChronoRun.cbp                 ← Code::Blocks project file
├── freeglut.dll                  ← Runtime DLL (must stay next to .exe)
│
├── build.bat                     ← One-click build script (MinGW g++)
├── run.bat                       ← One-click launcher
├── DEMO_VIDEO_INSTRUCTIONS.md    ← Guide for recording the demo video
└── README.md                     ← This file
```

---

## 🚀 How to Run

### Option 1 — Double-click (Easiest)
```
run.bat
```
Launches `ChronoRun.exe` directly. The `freeglut.dll` must be in the same folder (it already is).

### Option 2 — Build from Source
```bat
build.bat
```
Requires **MinGW g++** on your PATH (or installed at `C:\MinGW\bin\g++.exe`).  
Produces a fresh `ChronoRun.exe`.

### Option 3 — Code::Blocks IDE
Open `ChronoRun.cbp` and press **F9** to build & run.

### Option 4 — Play in Browser (Web Version)
Open `docs/index.html` directly in any browser, or start a local server:
```bat
python -m http.server 8000 --directory docs
```
Then visit: **http://localhost:8000**

---

## 🎮 Controls

| Key | Action |
| :--- | :--- |
| `SPACE` or `↑` | Jump (hold for higher jump) |
| `↓` | Duck / slide under drones |
| `T` | Cycle dinosaur theme (5 skins) |
| `P` | Pause / Resume |
| `R` | Restart |
| `H` | Toggle AABB hitbox debugger |
| `M` | Mute / unmute sound |
| `ESC` | Quit |
| Left Click | Jump |
| Right Click (hold) | Duck |

---

## 🦕 Dinosaur Themes (press `T` to switch)

| # | Theme Name | Body Color | Belly Color |
| :-: | :--- | :--- | :--- |
| 1 | Cyan Raptor *(default)* | Electric Blue | Golden Yellow |
| 2 | Jurassic Green | Emerald Green | Lime Yellow |
| 3 | Crimson Blaze | Fiery Red | Orange |
| 4 | Neon Cyber | Cyber Violet | Hot Magenta |
| 5 | Obsidian Gold | Charcoal | Amber Gold |

> Colors adapt automatically with the **day-night cycle**: the dino glows at night! 🌙

---

## 🎨 Graphics Concepts Demonstrated

| Concept | Implementation |
| :--- | :--- |
| 2D Orthographic Projection | `gluOrtho2D` |
| Raster Pixel-Art Sprites | `GL_QUADS` bitmap-to-quad renderer |
| Frame-Based Animation | Run cycle, jump, duck, crash poses |
| Linear Color Interpolation (`lerp`) | Day-night sky + dino ambient shading |
| Multi-Layer Parallax Scrolling | Far city skyline, near buildings, clouds, ground |
| AABB Collision Detection | Runner vs all obstacle types with live debugger |
| Delta-Time Physics | Frame-rate independent jump/gravity |
| Procedural Generation | Obstacles, buildings, clouds, stars |
| Synthesized Audio | In-memory WAV via `PlaySound` (jump, duck, score, game-over) |

---

## 🏙️ Obstacles

| Obstacle | Strategy |
| :--- | :--- |
| Traffic Cone (single / double) | Jump over |
| Road Barrier | Jump over |
| Bollard Cluster | Jump over |
| Delivery Drone (high) | Duck under |
| Delivery Drone (low) | Jump over |

---

## 📅 Development Timeline

| Dates | Milestone |
| :--- | :--- |
| Oct 26 – Nov 8 | Environment setup, rendering engine, jump physics, day-night cycle |
| Nov 9 – Nov 16 | Obstacle system, AABB collision, scoring, game states |
| Nov 17 – Nov 21 | Dinosaur runner + 5 dynamic themes, difficulty scaling, visual polish |
| Nov 22 – Nov 25 | Demo video, final submission archive, presentation rehearsal |

---

## 📦 Final Submission Checklist

- [x] Complete, commented `src/main.cpp`
- [x] Pre-compiled `ChronoRun.exe` (runs without installing anything)
- [x] Web browser version (`docs/index.html`)
- [x] PowerPoint presentation (`presentation/`)
- [x] `build.bat` one-click build script
- [ ] Demo video recording (2–3 min) — see `DEMO_VIDEO_INSTRUCTIONS.md`
- [ ] Final ZIP archive for Google Classroom submission

---

*Submitted for CSE-402 Computer Graphics & Image Processing Lab — Metropolitan University, Sylhet*
