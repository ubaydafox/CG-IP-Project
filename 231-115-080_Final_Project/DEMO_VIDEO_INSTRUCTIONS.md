# Demo Video Recording Guide (2–3 Minutes)
**Final Submission Requirement 4(a): Demo Video**  
**Course: CSE-402 Computer Graphics & Image Processing Lab**

---

## 1. Quick Recording with Free Windows Tools
You don't need any special software to record your demo video on Windows 10/11:

### Option A: Windows Snipping Tool (Built-in Screen Recorder)
1. Press `Win + Shift + R`.
2. Select the Chrono Run game window area.
3. Click **Record Audio / Mic** if you want to speak.
4. Click **Start** to record.
5. When done, click **Stop** and save as MP4.

### Option B: Windows Game Bar (Built-in)
1. Launch `ChronoRun.exe`.
2. Press `Win + G` to open the Xbox Game Bar.
3. Click the **Record** circle (or press `Win + Alt + R`).
4. Play the game and explain features for 2 minutes.
5. Press `Win + Alt + R` to stop. The video is saved to `C:\Users\<Name>\Videos\Captures`.

---

## 2. Recommended Video Script (2 Minutes)

- **0:00 - 0:25 | Introduction**:
  - Show the Start screen.
  - *"Hello everyone and Ma'am. We are Nahdiul Islam Rony and Abu Ubayda. This is our CSE-402 project: Chrono Run: DN Dash."*
  - Mention tools: C++, OpenGL, and FreeGLUT.

- **0:25 - 1:00 | Controls & Gameplay**:
  - Press `[SPACE]` to start running.
  - Demonstrate standard jumping over cacti and boulders.
  - Demonstrate ducking with `[DOWN ARROW]` when high-flying pterodactyls appear.
  - Mention variable jump height and particle effects on footfalls and landing.

- **1:00 - 1:40 | Dynamic Day-Night Cycle**:
  - Point out the sky changing from Dawn to bright Day, into Sunset, and then starry Night.
  - Point out the Sun rotating with rays, and the Moon with craters rising at night alongside twinkling stars.
  - Mention the linear color interpolation (`lerp`) technique used.

- **1:40 - 2:15 | Collision Detection & Hitbox Debugger**:
  - Press `[H]` to toggle on the green and red hitboxes live!
  - Explain the AABB (Axis-Aligned Bounding Box) algorithm.
  - Intentionally crash into an obstacle to demonstrate the radial particle explosion and Game Over screen with score comparison.

- **2:15 - 2:30 | Conclusion**:
  - Show the restart feature with `[R]`.
  - Conclude: *"Thank you for watching our demonstration!"*
