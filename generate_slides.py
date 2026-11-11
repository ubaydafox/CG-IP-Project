import sys
from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor
from pptx.enum.text import PP_ALIGN
from pptx.enum.shapes import MSO_SHAPE

def create_deck():
    prs = Presentation()
    prs.slide_width = Inches(13.333)
    prs.slide_height = Inches(7.5)
    blank_layout = prs.slide_layouts[6]

    # Color Palette: Modern Navy / Tech Dark Theme
    COLOR_BG = RGBColor(15, 23, 42)          # Deep slate navy
    COLOR_CARD = RGBColor(30, 41, 59)        # Card slate
    COLOR_PRIMARY = RGBColor(56, 189, 248)   # Cyan
    COLOR_ACCENT = RGBColor(251, 191, 36)    # Amber / Gold
    COLOR_WHITE = RGBColor(248, 250, 252)    # Clean white
    COLOR_MUTED = RGBColor(148, 163, 184)    # Cool gray
    COLOR_EMERALD = RGBColor(52, 211, 153)  # Emerald green

    def set_slide_background(slide):
        bg = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, 0, 0, prs.slide_width, prs.slide_height)
        bg.fill.solid()
        bg.fill.fore_color.rgb = COLOR_BG
        bg.line.fill.background()
        return bg

    def add_header(slide, title_text, category_text="CSE-402: COMPUTER GRAPHICS & IMAGE PROCESSING LAB"):
        # Category label
        tb_cat = slide.shapes.add_textbox(Inches(0.8), Inches(0.45), Inches(11.7), Inches(0.4))
        p_cat = tb_cat.text_frame.paragraphs[0]
        p_cat.text = category_text.upper()
        p_cat.font.size = Pt(11)
        p_cat.font.bold = True
        p_cat.font.color.rgb = COLOR_ACCENT

        # Title
        tb_title = slide.shapes.add_textbox(Inches(0.8), Inches(0.75), Inches(11.7), Inches(0.8))
        p_title = tb_title.text_frame.paragraphs[0]
        p_title.text = title_text
        p_title.font.size = Pt(26)
        p_title.font.bold = True
        p_title.font.color.rgb = COLOR_WHITE

        # Divider line
        line = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0.8), Inches(1.55), Inches(11.7), Inches(0.04))
        line.fill.solid()
        line.fill.fore_color.rgb = COLOR_PRIMARY
        line.line.fill.background()

    # -------------------------------------------------------------
    # SLIDE 1: Title Slide
    # -------------------------------------------------------------
    s1 = prs.slides.add_slide(blank_layout)
    set_slide_background(s1)

    # Accent top border
    top_bar = s1.shapes.add_shape(MSO_SHAPE.RECTANGLE, 0, 0, prs.slide_width, Inches(0.12))
    top_bar.fill.solid()
    top_bar.fill.fore_color.rgb = COLOR_PRIMARY
    top_bar.line.fill.background()

    # University Name
    tb_univ = s1.shapes.add_textbox(Inches(1.0), Inches(0.9), Inches(11.3), Inches(0.6))
    p_u = tb_univ.text_frame.paragraphs[0]
    p_u.text = "METROPOLITAN UNIVERSITY, SYLHET"
    p_u.font.size = Pt(14)
    p_u.font.bold = True
    p_u.font.color.rgb = COLOR_ACCENT

    # Main Project Title
    tb_mtitle = s1.shapes.add_textbox(Inches(1.0), Inches(1.5), Inches(11.3), Inches(1.5))
    p_mt = tb_mtitle.text_frame.paragraphs[0]
    p_mt.text = "CHRONO RUN: DN DASH"
    p_mt.font.size = Pt(44)
    p_mt.font.bold = True
    p_mt.font.color.rgb = COLOR_PRIMARY

    p_sub = tb_mtitle.text_frame.add_paragraph()
    p_sub.text = "An Interactive 2D Endless Runner with Dynamic Day-Night Simulation in OpenGL & C++"
    p_sub.font.size = Pt(18)
    p_sub.font.color.rgb = COLOR_WHITE
    p_sub.space_before = Pt(8)

    # Course info box
    tb_crs = s1.shapes.add_textbox(Inches(1.0), Inches(3.2), Inches(11.3), Inches(0.6))
    p_c = tb_crs.text_frame.paragraphs[0]
    p_c.text = "Course: Computer Graphics & Image Processing Lab (Course Code: CSE-402)"
    p_c.font.size = Pt(15)
    p_c.font.bold = True
    p_c.font.color.rgb = COLOR_EMERALD

    # Card 1: Submitted By
    c1 = s1.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(1.0), Inches(4.1), Inches(5.4), Inches(2.5))
    c1.fill.solid()
    c1.fill.fore_color.rgb = COLOR_CARD
    c1.line.color.rgb = COLOR_PRIMARY

    tb_subby = s1.shapes.add_textbox(Inches(1.2), Inches(4.2), Inches(5.0), Inches(2.3))
    tf1 = tb_subby.text_frame
    p1 = tf1.paragraphs[0]
    p1.text = "SUBMITTED BY:"
    p1.font.bold = True
    p1.font.size = Pt(13)
    p1.font.color.rgb = COLOR_ACCENT

    p1_m1 = tf1.add_paragraph()
    p1_m1.text = "• Nahdiul Islam Rony  (ID: 231-115-069)"
    p1_m1.font.size = Pt(13)
    p1_m1.font.color.rgb = COLOR_WHITE
    p1_m1.space_before = Pt(6)

    p1_m2 = tf1.add_paragraph()
    p1_m2.text = "• Abu Ubayda  (ID: 231-115-080)"
    p1_m2.font.size = Pt(13)
    p1_m2.font.color.rgb = COLOR_WHITE
    p1_m2.space_before = Pt(4)

    p1_sec = tf1.add_paragraph()
    p1_sec.text = "Section: B | 58th Batch\nDept. of Computer Science & Engineering"
    p1_sec.font.size = Pt(11)
    p1_sec.font.color.rgb = COLOR_MUTED
    p1_sec.space_before = Pt(6)

    # Card 2: Submitted To
    c2 = s1.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(6.9), Inches(4.1), Inches(5.4), Inches(2.5))
    c2.fill.solid()
    c2.fill.fore_color.rgb = COLOR_CARD
    c2.line.color.rgb = COLOR_PRIMARY

    tb_subto = s1.shapes.add_textbox(Inches(7.1), Inches(4.2), Inches(5.0), Inches(2.3))
    tf2 = tb_subto.text_frame
    p2 = tf2.paragraphs[0]
    p2.text = "SUBMITTED TO:"
    p2.font.bold = True
    p2.font.size = Pt(13)
    p2.font.color.rgb = COLOR_ACCENT

    p2_t = tf2.add_paragraph()
    p2_t.text = "Shrabanti Chowdhury"
    p2_t.font.bold = True
    p2_t.font.size = Pt(15)
    p2_t.font.color.rgb = COLOR_WHITE
    p2_t.space_before = Pt(8)

    p2_d = tf2.add_paragraph()
    p2_d.text = "Lecturer, Department of CSE\nMetropolitan University, Sylhet"
    p2_d.font.size = Pt(12)
    p2_d.font.color.rgb = COLOR_MUTED
    p2_d.space_before = Pt(4)

    # -------------------------------------------------------------
    # SLIDE 2: Project Overview & Objectives
    # -------------------------------------------------------------
    s2 = prs.slides.add_slide(blank_layout)
    set_slide_background(s2)
    add_header(s2, "Project Overview & Core Objectives")

    # Left Column: Project Concept
    card_l = s2.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(0.8), Inches(1.8), Inches(5.6), Inches(5.1))
    card_l.fill.solid()
    card_l.fill.fore_color.rgb = COLOR_CARD
    card_l.line.fill.background()

    tb_l = s2.shapes.add_textbox(Inches(1.0), Inches(2.0), Inches(5.2), Inches(4.7))
    tf_l = tb_l.text_frame
    tf_l.word_wrap = True
    p = tf_l.paragraphs[0]
    p.text = "WHAT IS CHRONO RUN?"
    p.font.bold = True
    p.font.size = Pt(14)
    p.font.color.rgb = COLOR_PRIMARY

    bullets_l = [
        "A 2D endless side-scrolling runner desktop game developed in C++ using pure OpenGL primitives and FreeGLUT.",
        "Inspired by classic runners (e.g. Chrome Dino), upgraded with dynamic real-time atmospheric rendering.",
        "Player controls a runner dodging procedural ground obstacles (cacti, rocks) and airborne threats (pterodactyls/cyber birds).",
        "Unique Core Novelty: Smooth real-time day-night cycle cycling between Dawn, Midday, Sunset, and Starry Night over 40 seconds."
    ]
    for b in bullets_l:
        p = tf_l.add_paragraph()
        p.text = "• " + b
        p.font.size = Pt(12)
        p.font.color.rgb = COLOR_WHITE
        p.space_before = Pt(10)

    # Right Column: Objectives
    card_r = s2.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(6.9), Inches(1.8), Inches(5.6), Inches(5.1))
    card_r.fill.solid()
    card_r.fill.fore_color.rgb = COLOR_CARD
    card_r.line.fill.background()

    tb_r = s2.shapes.add_textbox(Inches(7.1), Inches(2.0), Inches(5.2), Inches(4.7))
    tf_r = tb_r.text_frame
    tf_r.word_wrap = True
    p = tf_r.paragraphs[0]
    p.text = "KEY ACADEMIC OBJECTIVES"
    p.font.bold = True
    p.font.size = Pt(14)
    p.font.color.rgb = COLOR_ACCENT

    bullets_r = [
        "1. Primitive Modeling: Render character, terrain, celestial bodies, and obstacles strictly using OpenGL vector primitives.",
        "2. Dynamic Color Interpolation: Implement linear interpolation (lerp) across daytime color keyframes for fluid sky/ground shifts.",
        "3. Jump & Duck Physics: Model responsive velocity, gravity, and variable height jumping with crouch mechanics.",
        "4. Collision Detection: Implement real-time Axis-Aligned Bounding Box (AABB) checks with a toggleable visualizer.",
        "5. Procedural Generation: Scale game speed and spawn obstacles dynamically to maintain progressive difficulty."
    ]
    for b in bullets_r:
        p = tf_r.add_paragraph()
        p.text = b
        p.font.size = Pt(11.5)
        p.font.color.rgb = COLOR_WHITE
        p.space_before = Pt(9)

    # -------------------------------------------------------------
    # SLIDE 3: Computer Graphics Concepts Implemented
    # -------------------------------------------------------------
    s3 = prs.slides.add_slide(blank_layout)
    set_slide_background(s3)
    add_header(s3, "Computer Graphics Concepts Implemented")

    concepts = [
        ("2D Orthographic Projection", "Configured via gluOrtho2D(0, 1000, 0, 600) to map an intuitive coordinate system directly to screen pixels without perspective distortion.", COLOR_PRIMARY),
        ("Pure Vector Primitives", "Modeled models entirely using GL_QUADS, GL_TRIANGLE_FAN, GL_LINES, GL_POINTS, and GL_POLYGON without relying on external image textures.", COLOR_ACCENT),
        ("Color Interpolation (lerp)", "Smoothly blends sky RGB gradients across Dawn, Day, Sunset, and Night using: C = C1 + (C2 - C1) * t, updating every frame.", COLOR_EMERALD),
        ("Multi-Layer Parallax", "Distant mountains scroll at 0.12x, near hills at 0.35x, and clouds at 0.5x ground speed to produce realistic depth illusion.", COLOR_PRIMARY),
        ("Double Buffering & 60 FPS", "Configured with GLUT_DOUBLE | GLUT_RGB and glutTimerFunc(16ms) to eliminate screen tearing and maintain silky smooth rendering.", COLOR_ACCENT),
        ("Alpha Blending & Particles", "Enabled via glEnable(GL_BLEND) with GL_SRC_ALPHA for transparent HUD ribbons, sunset sun halos, and dust burst particles.", COLOR_EMERALD),
    ]

    for idx, (title, desc, col) in enumerate(concepts):
        col_idx = idx % 3
        row_idx = idx // 3
        x = Inches(0.8 + col_idx * 4.0)
        y = Inches(1.8 + row_idx * 2.6)

        box = s3.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, x, y, Inches(3.7), Inches(2.3))
        box.fill.solid()
        box.fill.fore_color.rgb = COLOR_CARD
        box.line.color.rgb = col
        box.line.width = Pt(1.5)

        tb = s3.shapes.add_textbox(x + Inches(0.15), y + Inches(0.15), Inches(3.4), Inches(2.0))
        tf = tb.text_frame
        tf.word_wrap = True

        p_t = tf.paragraphs[0]
        p_t.text = title
        p_t.font.bold = True
        p_t.font.size = Pt(13)
        p_t.font.color.rgb = col

        p_d = tf.add_paragraph()
        p_d.text = desc
        p_d.font.size = Pt(10.5)
        p_d.font.color.rgb = COLOR_WHITE
        p_d.space_before = Pt(6)

    # -------------------------------------------------------------
    # SLIDE 4: Dynamic Day-Night Cycle Architecture
    # -------------------------------------------------------------
    s4 = prs.slides.add_slide(blank_layout)
    set_slide_background(s4)
    add_header(s4, "Dynamic Day-Night Cycle Architecture")

    # 4 Phase Cards
    phases = [
        ("PHASE 1: DAWN (0 - 10s)", "• Sky shifts from deep indigo to warm golden-orange and coral.\n• Sun emerges from the left horizon with pulsing solar rays.\n• Ambient lighting factor increases from 0.40 to 0.85.\n• Morning mist and fading star visibility.", RGBColor(251, 146, 60)),
        ("PHASE 2: DAY (10 - 20s)", "• Radiant azure zenith blending into soft pale blue horizon.\n• Sun peaks at apex (Y = 480) with rotating ray geometry.\n• Full 1.0 ambient lighting across all terrain and obstacles.\n• Fluffy vector clouds drift freely across the blue sky.", RGBColor(56, 189, 248)),
        ("PHASE 3: SUNSET (20 - 30s)", "• Deep crimson and violet zenith fading to rich fiery amber.\n• Sun sinks toward the right horizon casting long warm tints.\n• Ambient light softens (0.75) with reddish environmental cast.\n• Twilight horizon prepares for nightfall.", RGBColor(244, 63, 94)),
        ("PHASE 4: NIGHT (30 - 40s)", "• Deep midnight navy sky with dark indigo horizon gradient.\n• Full glowing Moon with crater details rises across the sky.\n• 80+ stars twinkle with sine-wave modulated alpha.\n• Cool moonlight tint applied to runner and ground.", RGBColor(129, 140, 248)),
    ]

    for idx, (p_title, p_desc, p_col) in enumerate(phases):
        x = Inches(0.8 + idx * 2.95)
        y = Inches(1.8)

        box = s4.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, x, y, Inches(2.8), Inches(4.2))
        box.fill.solid()
        box.fill.fore_color.rgb = COLOR_CARD
        box.line.color.rgb = p_col
        box.line.width = Pt(1.5)

        tb = s4.shapes.add_textbox(x + Inches(0.12), y + Inches(0.15), Inches(2.55), Inches(3.9))
        tf = tb.text_frame
        tf.word_wrap = True

        pt = tf.paragraphs[0]
        pt.text = p_title
        pt.font.bold = True
        pt.font.size = Pt(12)
        pt.font.color.rgb = p_col

        pd = tf.add_paragraph()
        pd.text = p_desc
        pd.font.size = Pt(10.5)
        pd.font.color.rgb = COLOR_WHITE
        pd.space_before = Pt(8)

    # Bottom summary ribbon
    bot_card = s4.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(0.8), Inches(6.2), Inches(11.7), Inches(0.85))
    bot_card.fill.solid()
    bot_card.fill.fore_color.rgb = COLOR_CARD
    bot_card.line.fill.background()

    tb_bot = s4.shapes.add_textbox(Inches(1.0), Inches(6.25), Inches(11.3), Inches(0.75))
    tf_b = tb_bot.text_frame
    p_b = tf_b.paragraphs[0]
    p_b.text = "Mathematical Foundation: Cyclic parameter t = (cycleTimer % 40) / 40. Celestial paths follow circular parametric equations: x = cx + r*cos(θ), y = cy + r*sin(θ), producing realistic orbital movement."
    p_b.font.size = Pt(11)
    p_b.font.color.rgb = COLOR_ACCENT

    # -------------------------------------------------------------
    # SLIDE 5: Physics, Jump Mechanics & Collision Detection
    # -------------------------------------------------------------
    s5 = prs.slides.add_slide(blank_layout)
    set_slide_background(s5)
    add_header(s5, "Physics, Jump Mechanics & Collision Detection")

    # Column 1: Jump & Duck Physics
    c1 = s5.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(0.8), Inches(1.8), Inches(5.6), Inches(5.1))
    c1.fill.solid()
    c1.fill.fore_color.rgb = COLOR_CARD
    c1.line.fill.background()

    tb1 = s5.shapes.add_textbox(Inches(1.0), Inches(2.0), Inches(5.2), Inches(4.7))
    tf1 = tb1.text_frame
    tf1.word_wrap = True

    p = tf1.paragraphs[0]
    p.text = "RUNNER PHYSICS & DYNAMICS"
    p.font.bold = True
    p.font.size = Pt(14)
    p.font.color.rgb = COLOR_PRIMARY

    physics_notes = [
        "Impulse & Gravity: Jumping triggers instantaneous velocity impulse (JUMP_FORCE = 15.2f), steadily dampened by constant gravity (GRAVITY = 0.78f).",
        "Variable Jump Height: Releasing the Spacebar or Up Arrow early truncates upward velocity by 45%, providing platformer-grade tactile control.",
        "Ducking / Sliding: Pressing Down Arrow compresses the character vertically (Height: 70px -> 38px, Width: 44px -> 64px) to slide beneath high birds.",
        "Dust Emitter: Dynamic particle generation upon jump takeoff, footfalls, and landing impact for kinetic game feedback."
    ]
    for n in physics_notes:
        p = tf1.add_paragraph()
        p.text = "• " + n
        p.font.size = Pt(11.5)
        p.font.color.rgb = COLOR_WHITE
        p.space_before = Pt(8)

    # Column 2: Collision Detection
    c2 = s5.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(6.9), Inches(1.8), Inches(5.6), Inches(5.1))
    c2.fill.solid()
    c2.fill.fore_color.rgb = COLOR_CARD
    c2.line.fill.background()

    tb2 = s5.shapes.add_textbox(Inches(7.1), Inches(2.0), Inches(5.2), Inches(4.7))
    tf2 = tb2.text_frame
    tf2.word_wrap = True

    p = tf2.paragraphs[0]
    p.text = "AABB COLLISION DETECTION & HITBOXES"
    p.font.bold = True
    p.font.size = Pt(14)
    p.font.color.rgb = COLOR_EMERALD

    col_notes = [
        "Algorithm: Axis-Aligned Bounding Box (AABB) intersection check executed every frame:",
        "    collision = (a.x < b.x + b.w) && (a.x + a.w > b.x) &&\n                (a.y < b.y + b.h) && (a.y + a.h > b.y);",
        "Fair Hitbox Inset: 6px horizontal and 4px vertical padding ensures player satisfaction without unfair edge catches.",
        "Debug Hitbox Visualizer (Press 'H'): Renders bright green bounding box for runner and red bounding boxes for obstacles—ideal for lab presentation & grading!",
        "Impact Explosion: Collision triggers a 40-particle radial blast, Game Over overlay, and pitch-descending sound cue."
    ]
    for n in col_notes:
        p = tf2.add_paragraph()
        p.text = n if n.startswith(" ") else "• " + n
        p.font.size = Pt(11)
        p.font.color.rgb = COLOR_ACCENT if "collision =" in n else COLOR_WHITE
        p.space_before = Pt(6)

    # -------------------------------------------------------------
    # SLIDE 6: Procedural Obstacles & Gameplay Variety
    # -------------------------------------------------------------
    s6 = prs.slides.add_slide(blank_layout)
    set_slide_background(s6)
    add_header(s6, "Procedural Obstacles & Gameplay Progression")

    obs_list = [
        ("Small Cactus", "Ground single trunk with dual staggered arms (GL_QUADS & GL_TRIANGLE_FAN). Easily cleared with standard jump.", RGBColor(52, 211, 153)),
        ("Double Cacti", "Wider ground hurdle requiring precise jump timing to clear both cacti heads without clipping.", RGBColor(34, 197, 94)),
        ("Tall Giant Cactus", "Taller profile testing maximum jump apex and airborne clearance at high runner speeds.", RGBColor(16, 185, 129)),
        ("Jagged Boulder", "Low triangular rock cluster with multi-faceted shading, blending against desert bedrock.", RGBColor(245, 158, 11)),
        ("Low Flying Bird", "Airborne pterodactyl flapping wings (sin(t)*16px) at knee level. Must be jumped over!", RGBColor(239, 68, 68)),
        ("High Flying Bird", "Flies at head level (Y = 154px). Standing or jumping causes a hit—player MUST duck to survive!", RGBColor(168, 85, 247))
    ]

    for idx, (title, desc, col) in enumerate(obs_list):
        c_i = idx % 3
        r_i = idx // 3
        x = Inches(0.8 + c_i * 4.0)
        y = Inches(1.8 + r_i * 2.6)

        box = s6.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, x, y, Inches(3.7), Inches(2.3))
        box.fill.solid()
        box.fill.fore_color.rgb = COLOR_CARD
        box.line.color.rgb = col
        box.line.width = Pt(1.5)

        tb = s6.shapes.add_textbox(x + Inches(0.15), y + Inches(0.15), Inches(3.4), Inches(2.0))
        tf = tb.text_frame
        tf.word_wrap = True

        pt = tf.paragraphs[0]
        pt.text = title
        pt.font.bold = True
        pt.font.size = Pt(13)
        pt.font.color.rgb = col

        pd = tf.add_paragraph()
        pd.text = desc
        pd.font.size = Pt(10.5)
        pd.font.color.rgb = COLOR_WHITE
        pd.space_before = Pt(6)

    # -------------------------------------------------------------
    # SLIDE 7: UI, HUD & Audio System
    # -------------------------------------------------------------
    s7 = prs.slides.add_slide(blank_layout)
    set_slide_background(s7)
    add_header(s7, "HUD, Game States & Asynchronous Audio")

    # Left: UI & HUD
    cl = s7.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(0.8), Inches(1.8), Inches(5.6), Inches(5.1))
    cl.fill.solid()
    cl.fill.fore_color.rgb = COLOR_CARD
    cl.line.fill.background()

    tbl = s7.shapes.add_textbox(Inches(1.0), Inches(2.0), Inches(5.2), Inches(4.7))
    tfl = tbl.text_frame
    tfl.word_wrap = True
    p = tfl.paragraphs[0]
    p.text = "HUD & GAME STATE MACHINE"
    p.font.bold = True
    p.font.size = Pt(14)
    p.font.color.rgb = COLOR_PRIMARY

    ui_points = [
        "Real-Time HUD Ribbon: Clean translucent top bar displaying current score (SCORE: 00340), high score (HI: 01250), speed multiplier (SPEED: 1.4x), and active time state (DAY / SUNSET / NIGHT).",
        "State Machine Architecture: Handles 4 distinct game states: STATE_START, STATE_PLAYING, STATE_PAUSED, and STATE_GAME_OVER.",
        "Interactive Overlays: Polished pop-up dialogs for Start instructions, Pause screen ('Press P to Resume'), and Game Over with high score badge.",
        "Bitmap Typography: Crisp on-screen text drawn using glutBitmapCharacter() with scalable Helvetica and Times Roman fonts."
    ]
    for pt in ui_points:
        p = tfl.add_paragraph()
        p.text = "• " + pt
        p.font.size = Pt(11.5)
        p.font.color.rgb = COLOR_WHITE
        p.space_before = Pt(8)

    # Right: Audio & Controls
    cr = s7.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(6.9), Inches(1.8), Inches(5.6), Inches(5.1))
    cr.fill.solid()
    cr.fill.fore_color.rgb = COLOR_CARD
    cr.line.fill.background()

    tbr = s7.shapes.add_textbox(Inches(7.1), Inches(2.0), Inches(5.2), Inches(4.7))
    tfr = tbr.text_frame
    tfr.word_wrap = True
    p = tfr.paragraphs[0]
    p.text = "NON-BLOCKING AUDIO & CONTROLS"
    p.font.bold = True
    p.font.size = Pt(14)
    p.font.color.rgb = COLOR_ACCENT

    audio_points = [
        "Threaded Audio System: Uses std::thread with Windows Beep() to generate retro arcade sound effects without freezing or stalling the 60 FPS graphics loop.",
        "Sound Cues: Distinct audio frequencies for jump chirp (520-780Hz), duck click (330Hz), 100-point milestone fanfare (880-1174Hz), and crash descend.",
        "Comprehensive Keyboard Controls:\n  - [SPACEBAR] or [UP ARROW]: Jump\n  - [DOWN ARROW]: Duck / Slide\n  - [P]: Pause / Resume\n  - [R]: Quick Restart\n  - [H]: Toggle Debug Hitbox Outlines\n  - [M]: Mute / Unmute Audio\n  - [ESC]: Exit Game"
    ]
    for pt in audio_points:
        p = tfr.add_paragraph()
        p.text = pt if pt.startswith("Threaded") or pt.startswith("Sound") else pt
        p.font.size = Pt(11)
        p.font.color.rgb = COLOR_WHITE
        p.space_before = Pt(8)

    # -------------------------------------------------------------
    # SLIDE 8: Team Responsibilities & Work Division
    # -------------------------------------------------------------
    s8 = prs.slides.add_slide(blank_layout)
    set_slide_background(s8)
    add_header(s8, "Team Responsibilities & Work Division")

    # Member 1 Card
    m1_box = s8.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(0.8), Inches(1.8), Inches(5.6), Inches(4.2))
    m1_box.fill.solid()
    m1_box.fill.fore_color.rgb = COLOR_CARD
    m1_box.line.color.rgb = COLOR_PRIMARY
    m1_box.line.width = Pt(2)

    tb_m1 = s8.shapes.add_textbox(Inches(1.0), Inches(2.0), Inches(5.2), Inches(3.8))
    tf_m1 = tb_m1.text_frame
    tf_m1.word_wrap = True
    p = tf_m1.paragraphs[0]
    p.text = "MEMBER 1: NAHDIUL ISLAM RONY"
    p.font.bold = True
    p.font.size = Pt(13)
    p.font.color.rgb = COLOR_PRIMARY

    p_id1 = tf_m1.add_paragraph()
    p_id1.text = "Student ID: 231-115-069 | Section B, 58th Batch"
    p_id1.font.size = Pt(11)
    p_id1.font.color.rgb = COLOR_MUTED
    p_id1.space_before = Pt(2)

    m1_tasks = [
        "Graphics Rendering Pipeline: OpenGL context setup, orthographic viewport projection, and matrix mode setup.",
        "Day-Night Cycle Engine: Sky color gradient interpolation (lerp), celestial orbital arcs for Sun and Moon, twinkling star field.",
        "Vector Character & Obstacle Art: Modeling runner animations (running cycle, jump pose, duck slide) and obstacle vector geometry.",
        "Parallax Background: Multi-layer depth scrolling (far mountains, near hills, drifting clouds, ground pebbles)."
    ]
    for t in m1_tasks:
        p = tf_m1.add_paragraph()
        p.text = "• " + t
        p.font.size = Pt(11)
        p.font.color.rgb = COLOR_WHITE
        p.space_before = Pt(6)

    # Member 2 Card
    m2_box = s8.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(6.9), Inches(1.8), Inches(5.6), Inches(4.2))
    m2_box.fill.solid()
    m2_box.fill.fore_color.rgb = COLOR_CARD
    m2_box.line.color.rgb = COLOR_ACCENT
    m2_box.line.width = Pt(2)

    tb_m2 = s8.shapes.add_textbox(Inches(7.1), Inches(2.0), Inches(5.2), Inches(3.8))
    tf_m2 = tb_m2.text_frame
    tf_m2.word_wrap = True
    p = tf_m2.paragraphs[0]
    p.text = "MEMBER 2: ABU UBAYDA"
    p.font.bold = True
    p.font.size = Pt(13)
    p.font.color.rgb = COLOR_ACCENT

    p_id2 = tf_m2.add_paragraph()
    p_id2.text = "Student ID: 231-115-080 | Section B, 58th Batch"
    p_id2.font.size = Pt(11)
    p_id2.font.color.rgb = COLOR_MUTED
    p_id2.space_before = Pt(2)

    m2_tasks = [
        "Collision Detection (AABB): Mathematical boundary intersection logic, hitbox fine-tuning, and interactive debug visualizer.",
        "Physics & Jump Controller: Impulse force, gravity decay, variable height jump dampening, and crouching slide mechanics.",
        "Procedural Obstacle Generation: Dynamic distance spawning, difficulty speed curves, and fair obstacle gap calculations.",
        "HUD & State Management: Start/Pause/Game-Over states, high-score tracking, non-blocking multi-threaded sound system."
    ]
    for t in m2_tasks:
        p = tf_m2.add_paragraph()
        p.text = "• " + t
        p.font.size = Pt(11)
        p.font.color.rgb = COLOR_WHITE
        p.space_before = Pt(6)

    # Shared bottom banner
    shared_box = s8.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(0.8), Inches(6.2), Inches(11.7), Inches(0.85))
    shared_box.fill.solid()
    shared_box.fill.fore_color.rgb = COLOR_CARD
    shared_box.line.fill.background()

    tb_sh = s8.shapes.add_textbox(Inches(1.0), Inches(6.25), Inches(11.3), Inches(0.75))
    tf_sh = tb_sh.text_frame
    p_sh = tf_sh.paragraphs[0]
    p_sh.text = "Joint Responsibilities: System integration, Code::Blocks build configuration, gameplay playtesting & balancing, README documentation, demo video recording, and slide deck preparation."
    p_sh.font.size = Pt(11)
    p_sh.font.color.rgb = COLOR_EMERALD

    # -------------------------------------------------------------
    # SLIDE 9: Project Timeline & Deliverables Status
    # -------------------------------------------------------------
    s9 = prs.slides.add_slide(blank_layout)
    set_slide_background(s9)
    add_header(s9, "Project Timeline & Lab Milestone Deliverables")

    milestones = [
        ("STAGE 1: PROPOSAL", "25 Oct 2026", "COMPLETED", "• Project title: Chrono Run: DN Dash\n• Objectives & scope defined\n• Team formation approved (Nahdiul & Ubayda)\n• Hardcopy & softcopy submitted", COLOR_EMERALD),
        ("STAGE 2: PROGRESS UPDATE", "8 Nov 2026", "ON TRACK", "• In-person lab demonstration\n• Working 2D primitive rendering\n• Smooth day-night color cycle\n• Functional jump & duck physics", COLOR_PRIMARY),
        ("STAGE 3: FINAL SUBMISSION", "25 Nov 2026", "READY", "• a) Complete & runnable C++ source code\n• b) High-resolution Demo Video\n• c) Professional Presentation Slides (PPT)\n• StudentID_Final_Project archive", COLOR_ACCENT)
    ]

    for idx, (stage, date, status, details, col) in enumerate(milestones):
        x = Inches(0.8 + idx * 4.0)
        y = Inches(1.8)

        box = s9.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, x, y, Inches(3.7), Inches(5.1))
        box.fill.solid()
        box.fill.fore_color.rgb = COLOR_CARD
        box.line.color.rgb = col
        box.line.width = Pt(2)

        tb = s9.shapes.add_textbox(x + Inches(0.15), y + Inches(0.15), Inches(3.4), Inches(4.7))
        tf = tb.text_frame
        tf.word_wrap = True

        pt = tf.paragraphs[0]
        pt.text = stage
        pt.font.bold = True
        pt.font.size = Pt(13)
        pt.font.color.rgb = col

        p_d = tf.add_paragraph()
        p_d.text = f"Deadline: {date}  [{status}]"
        p_d.font.bold = True
        p_d.font.size = Pt(11)
        p_d.font.color.rgb = COLOR_WHITE
        p_d.space_before = Pt(4)

        p_det = tf.add_paragraph()
        p_det.text = details
        p_det.font.size = Pt(11)
        p_det.font.color.rgb = COLOR_MUTED
        p_det.space_before = Pt(12)

    # -------------------------------------------------------------
    # SLIDE 10: Conclusion & Live Demonstration
    # -------------------------------------------------------------
    s10 = prs.slides.add_slide(blank_layout)
    set_slide_background(s10)
    add_header(s10, "Conclusion & Live Project Demonstration")

    card_c = s10.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(1.5), Inches(1.8), Inches(10.3), Inches(5.1))
    card_c.fill.solid()
    card_c.fill.fore_color.rgb = COLOR_CARD
    card_c.line.color.rgb = COLOR_PRIMARY
    card_c.line.width = Pt(2)

    tbc = s10.shapes.add_textbox(Inches(1.8), Inches(2.1), Inches(9.7), Inches(4.5))
    tfc = tbc.text_frame
    tfc.word_wrap = True

    p = tfc.paragraphs[0]
    p.text = "SUMMARY OF KEY TAKEAWAYS"
    p.font.bold = True
    p.font.size = Pt(16)
    p.font.color.rgb = COLOR_PRIMARY

    bullets_c = [
        "End-to-End Graphics Pipeline: Practical mastery of 2D coordinate transformation, projection matrix setup, and real-time double-buffered animation.",
        "Dynamic Vector Aesthetics: Proved that engaging visual atmosphere can be achieved with pure vector primitives through intelligent color interpolation.",
        "Robust Interactive Physics: Balanced responsive jump/duck dynamics with precision AABB bounding box collision checks.",
        "Modular & Portable Engineering: Zero-dependency executable with embedded FreeGLUT configuration ready for instant compilation and execution."
    ]
    for b in bullets_c:
        p = tfc.add_paragraph()
        p.text = "• " + b
        p.font.size = Pt(12.5)
        p.font.color.rgb = COLOR_WHITE
        p.space_before = Pt(10)

    p_demo = tfc.add_paragraph()
    p_demo.text = "NOW COMMENCING LIVE DEMONSTRATION & Q&A"
    p_demo.font.bold = True
    p_demo.font.size = Pt(16)
    p_demo.font.color.rgb = COLOR_ACCENT
    p_demo.space_before = Pt(24)

    p_thanks = tfc.add_paragraph()
    p_thanks.text = "Thank you!  |  Questions & Feedback are warmly welcome."
    p_thanks.font.size = Pt(13)
    p_thanks.font.color.rgb = COLOR_EMERALD
    p_thanks.space_before = Pt(6)

    prs.save("Chrono_Run_DN_Dash_Presentation.pptx")
    print("Presentation saved successfully: Chrono_Run_DN_Dash_Presentation.pptx")

if __name__ == "__main__":
    create_deck()
