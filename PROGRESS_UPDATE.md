# CG&IP Lab (CSE-402) — Progress Update Preparation Guide
**Milestone Date: Sunday, 8 November 2026 (In-Person Demonstration)**  
**Lecturer: Shrabanti Chowdhury, Department of CSE, Metropolitan University, Sylhet**

---

## 1. Quick Demonstration Checklist
Before entering the lab:
1. Copy the project folder to a laptop or USB drive.
2. Test double-clicking `run.bat` to verify the game launches smoothly without opening errors.
3. Open `ChronoRun.cbp` in Code::Blocks and press **F9** to ensure it builds and runs in front of the teacher.

---

## 2. In-Person Talking Points (What to Say)

### Opening Statement:
> *"Good morning / afternoon Ma'am. We are Nahdiul Islam Rony and Abu Ubayda from Section B (58th Batch). Today we are demonstrating our ongoing progress for our project: **Chrono Run: DN Dash**, an interactive 2D endless runner built using C++, OpenGL, and FreeGLUT."*

### Key Features to Demonstrate Live:
1. **Dynamic Day-Night Cycle**:
   - Point out the sky color transition: *"Notice how the background sky smoothly transitions from dawn into midday, then into a deep sunset, and finally into a starry night over 40 seconds using RGB linear color interpolation."*
   - Point out the celestial bodies: *"The Sun follows a circular arc across the sky with rotating rays, and the Moon with craters rises at night alongside 80+ twinkling stars."*
2. **Jump & Ducking Physics**:
   - Press `[SPACE]` to jump over cones, barriers and bollards: *"We implemented realistic jump velocity and gravity deceleration. If we tap lightly, the jump is short; if we hold, it reaches full height."*
   - Press `[DOWN]` to duck: *"When high-flying delivery drones appear, we can duck down into a low slide pose with reduced hitbox height, or fast-fall in mid-air."*
3. **Collision Detection & Hitbox Debugger**:
   - **Press `[H]` to show hitboxes!**: *"Ma'am, we added an interactive debug visualizer by pressing 'H'. The green box is our player hitbox and red boxes are obstacle hitboxes. We use Axis-Aligned Bounding Box (AABB) intersection mathematics to detect collisions."*
4. **Scoring, Difficulty Scaling & Sound**:
   - *"As distance increases, the game speed scales up progressively, and milestone chimes play every 100 points via non-blocking background threads."*

---

## 3. Division of Work Explained

### Nahdiul Islam Rony (ID: 231-115-069):
- Implemented the OpenGL rendering pipeline and coordinate projection (`gluOrtho2D`).
- Modeled the 2D vector primitives (`GL_QUADS`, `GL_TRIANGLE_FAN`, `GL_LINES`, `GL_POLYGON`).
- Developed the day-night color interpolation logic and celestial motion mathematics.
- Designed multi-layer parallax scrolling (mountains, hills, clouds, and ground details).

### Abu Ubayda (ID: 231-115-080):
- Implemented the AABB collision detection algorithm and debug hitbox toggle (`[H]`).
- Engineered the jump velocity, gravity physics, and ducking hitbox compression.
- Built procedural obstacle spawning with dynamic gap spacing and progressive speed scaling.
- Programmed HUD score tracking, game state management (Start/Play/Pause/Game-Over), and audio system.

---

## 4. What is Completed vs What is Remaining

### What is Completed (Current Status: ~90%):
- [x] Complete OpenGL graphics engine and orthographic viewport.
- [x] Full 4-phase dynamic day-night cycle with color interpolation.
- [x] Animated player runner (running leg swing, arm pump, headband streamer, jump pose, duck slide).
- [x] 6 distinct obstacle types (small cactus, double cacti, tall cactus, rock boulder, low bird, high bird).
- [x] AABB collision detection and debug visualizer.
- [x] Real-time HUD, score, high score, and pause/restart states.
- [x] Presentation slide deck (`Chrono_Run_DN_Dash_Presentation.pptx`).

### What is Remaining (For Final Submission on 25 Nov 2026):
- [ ] Record the 2–3 minute **Demo Video** with voiceover explaining the features.
- [ ] Prepare final Google Classroom submission archive (`StudentID_Final_Project.zip`).
- [ ] Practice the PowerPoint presentation slides for the final lab defense.
