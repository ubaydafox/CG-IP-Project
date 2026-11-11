# CHRONO RUN: DN DASH
**An Interactive 2D Endless Runner with Real-Time Day-Night Simulation**

**Course**: Computer Graphics & Image Processing Lab (Course Code: `CSE-402`)  
**Department**: Department of Computer Science & Engineering, Metropolitan University, Sylhet  
**Course Lecturer**: Shrabanti Chowdhury  

---

### Team Information (Section B, 58th Batch)
- **Nahdiul Islam Rony** — Student ID: `231-115-069`
- **Abu Ubayda** — Student ID: `231-115-080`

---

## 1. Project Overview & Motivation
**Chrono Run: DN Dash** is a high-performance 2D endless runner desktop game developed in **C++** using **OpenGL** and **FreeGLUT**. Inspired by retro endless runners like the Chrome Dino game, the project demonstrates core computer graphics concepts such as 2D orthographic projection, vector primitive modeling, multi-layer parallax depth scrolling, frame-rate-independent physics, linear color interpolation (`lerp`), and Axis-Aligned Bounding Box (AABB) collision detection.

### Unique Feature: Dynamic Day-Night Cycle
The standout feature is a seamless, real-time day-night cycle spanning four distinct atmospheric phases:
1. **Dawn (0–10s)**: Soft pink and coral horizon, rising golden Sun, fading morning stars, and warming ambient lighting.
2. **Midday (10–20s)**: Vibrant azure blue sky, radiant Sun with animated rotating solar rays, and drifting fluffy vector clouds.
3. **Sunset (20–30s)**: Deep crimson and violet dusk gradients, sinking Sun casting warm red tints across the desert terrain.
4. **Starry Night (30–40s)**: Deep midnight navy sky, full glowing Moon with crater details, and 80+ twinkling stars modulated by sine-wave functions.

---

## 2. Gameplay Mechanics & Controls

| Key / Control | In-Game Action | Mechanics & Description |
| :--- | :--- | :--- |
| `[SPACEBAR]` or `[UP ARROW]` | **Jump** | Variable-height jump. Tap for short hop; hold for full jump apex. |
| `[DOWN ARROW]` | **Duck / Slide** | Low slide crouch. Compresses hitbox from 70px to 38px to slide under high birds. |
| `[P]` | **Pause / Resume** | Freezes simulation timer and displays pause dialog. |
| `[R]` | **Restart** | Instantly resets score, speed, obstacles, and player position. |
| `[H]` | **Hitbox Visualizer** | **Lab Feature**: Toggles green (runner) and red (obstacles) collision boundary boxes. |
| `[M]` | **Sound On / Off** | Toggles the gentle sound effects. |
| Mouse | **Jump / Duck** | Left click = jump, hold right click = duck. |
| `[ESC]` | **Exit** | Closes the OpenGL window gracefully. |

---

## 3. Project Structure
```
d:\CG&IP project\
├── src\
│   └── main.cpp           # Complete, well-documented C++ OpenGL game source code
├── include\
│   └── GL\
│       ├── freeglut.h     # FreeGLUT API header
│       ├── freeglut_ext.h
│       ├── freeglut_std.h
│       └── glut.h
├── lib\
│   └── libfreeglut.a      # 64-bit MinGW FreeGLUT library
├── freeglut.dll           # Runtime dynamic library (placed alongside executable)
├── ChronoRun.exe          # Precompiled ready-to-run Windows executable
├── ChronoRun.cbp          # Ready-to-open Code::Blocks Project File
├── build.bat              # One-click command-line build script using MinGW g++
├── run.bat                # One-click game launcher script
├── generate_slides.py     # Python script to generate the presentation slide deck
├── Chrono_Run_DN_Dash_Presentation.pptx # Final Presentation Slides (PPT)
├── PROGRESS_UPDATE.md     # Progress report & talking points for Nov 8 milestone
└── README.md              # Project documentation and run instructions
```

---

## 4. How to Build and Run

### Method A: One-Click Batch Script (Recommended & Fastest)
1. **To Run immediately**: Double-click `run.bat` (or double-click `ChronoRun.exe`).
2. **To Recompile**: Double-click `build.bat`. It automatically detects MinGW / Code::Blocks `g++` and compiles `src/main.cpp` into `ChronoRun.exe`.

### Method B: Using Code::Blocks IDE
1. Open **Code::Blocks**.
2. Go to **File -> Open...** and select `ChronoRun.cbp`.
3. Press **F9** (or click **Build -> Build and run**).
4. The game will compile and launch immediately (compiler include and linker paths are pre-configured).

### Method C: Command-Line Compilation (MinGW GCC)
Open PowerShell or Command Prompt in the project folder and run:
```bash
g++ -O2 -static -I"include" -L"lib" src/main.cpp -lfreeglut -lopengl32 -lglu32 -lwinmm -o ChronoRun.exe
./ChronoRun.exe
```
*(If `g++` is not in your system PATH, use `"C:\Program Files\CodeBlocks\MinGW\bin\g++.exe"`)*.

### Method D: Mobile / Browser Version (Android, iPhone, any browser)
A Canvas/JavaScript port of the same game is in [`docs/index.html`](docs/index.html).
- **On a PC**: just double-click `docs/index.html` to play in your browser.
- **On a phone**: enable GitHub Pages (repo **Settings → Pages → Branch: `main`, folder: `/docs`**), then open
  `https://ubaydafox.github.io/CG-IP-Project/` on your phone. Use **Share → Add to Home Screen** to install it like an app.
- **Touch controls**: tap anywhere or the **JUMP** button to jump (hold for a higher jump), hold **DUCK** to duck,
  plus on-screen **pause** and **sound** buttons at the top. Rotate to landscape for the best view.

### Sound
Gentle, soft sine-tone effects (jump, duck, every 100 points, game over). Press `[M]` (or the sound button on mobile) to mute.

---

## 5. Computer Graphics Concepts Implemented (For Lab Evaluation)

1. **2D Orthographic Projection**:  
   Set up via `gluOrtho2D(0.0, 1000.0, 0.0, 600.0)` in the reshape callback to create a 1:1 pixel coordinate system.
2. **Vector Primitive Modeling**:  
   Character, terrain, celestial bodies, and obstacles are drawn purely with vector primitives:
   - `GL_QUADS`: Ground base, torso, visor, cacti trunks, building blocks.
   - `GL_TRIANGLE_FAN`: Sun, Moon, head, craters, cloud puffs, dust particles.
   - `GL_LINES`: Sun rays, star twinkling, ground surface line, grass blades.
   - `GL_POLYGON` & `GL_TRIANGLES`: Jagged boulders, headband streamers, flapping bird wings.
3. **Linear Color Interpolation (`lerp`)**:  
   Computes $C(t) = C_1 + (C_2 - C_1) \times t$ across daytime keyframe palettes to blend sky colors and environmental tinting every frame without visual banding.
4. **Multi-Layer Parallax Scrolling**:  
   Simulates 3D depth on a 2D plane by scrolling background layers at varied rates:
   - Distant Mountains: $0.12 \times \text{Speed}$
   - Near Hills: $0.35 \times \text{Speed}$
   - Floating Clouds: $0.50 \times \text{Speed}$
   - Ground Terrain: $1.00 \times \text{Speed}$
5. **Double Buffering & 60 FPS Fixed Time-step**:  
   Configured using `GLUT_DOUBLE | GLUT_RGB` and `glutTimerFunc(16, ...)` to ensure tear-free, smooth rendering.
6. **Alpha Blending & Particle Physics**:  
   Enabled via `glEnable(GL_BLEND)` with `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)` for translucent HUD ribbons, dust bursts on footfalls/landing, and radial crash explosions.
7. **Axis-Aligned Bounding Box (AABB) Collision Detection**:  
   Real-time mathematical intersection checks with calibrated hitboxes. Pressing `[H]` reveals debug collision frames live.

---

## 6. Team Work Division

| Task / Responsibility | Primary Member | Contribution Details |
| :--- | :--- | :--- |
| **Graphics Rendering & Shading** | **Nahdiul Islam Rony** (231-115-069) | OpenGL context setup, orthographic projection, vector primitive drawing, dynamic day-night color interpolation, and parallax background scrolling. |
| **Physics, Collision & Gameplay** | **Abu Ubayda** (231-115-080) | AABB collision algorithm, jump velocity & gravity physics, duck mechanics, procedural obstacle generation, speed progression, and HUD text rendering. |
| **Integration & Presentation** | **Both Members** | Playtesting, difficulty balancing, Code::Blocks integration, README documentation, demo video, and PowerPoint presentation slides. |

---

## 7. Submission Checklist (Teacher Instructions)
- [x] **Project Proposal**: Submitted on Oct 26, 2026 (`StudentID_Proposal.pdf`).
- [x] **Progress Update (8 Nov 2026)**: In-person demonstration prepared with `PROGRESS_UPDATE.md`.
- [x] **Source Code**: Complete, runnable, fully commented in `src/main.cpp`.
- [x] **README File**: Clear build and execution instructions (this file).
- [x] **Presentation Slides**: `Chrono_Run_DN_Dash_Presentation.pptx` (10 widescreen slides).
- [ ] **Demo Video**: Record a 2–3 minute video showing gameplay, jump/duck controls, dynamic day-night cycle, and hitbox toggle (`[H]`).
- [ ] **Google Classroom Upload**: Submit archive named `231-115-080_Final_Project.zip` before Nov 25, 2026 (11:59 PM).
