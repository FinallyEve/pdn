# PDN UI Design Option B: Bold Retro

## Design Philosophy

**Bold Retro** embraces chunky, high-contrast visuals inspired by 1980s arcade games and early computer interfaces. Every element is designed for maximum readability and visual impact on the 128x64 OLED display.

### Core Principles
- **Heavy Contrast**: Thick borders, inverted text blocks, solid fills
- **Dense Information**: Pack status data efficiently using Siji icon font
- **Pixel-Art Aesthetic**: Chunky, retro-futuristic appearance
- **Clear Hierarchy**: Full-width header bars separate content zones
- **Bold Typography**: Use 6x13B or similar bold bitmap fonts

### Font Palette
- **Headers**: u8g2_font_6x13B_tr (bold 6x13, inverted blocks)
- **Body Text**: u8g2_font_6x10_tr (readable 6x10)
- **Large Text**: u8g2_font_9x15B_tr (bold 9x15 for emphasis)
- **Icons**: u8g2_font_siji_t_6x10 (6x10 status icons - PREFERRED)
- **2x Icons**: u8g2_font_open_iconic_play_2x_t (16x16 for game symbols)

### Color Modes
- DrawColor 1 = White pixels (default)
- DrawColor 0 = Black pixels (for inverted text)
- DrawColor 2 = XOR (for highlights)

---

## Screen Mockups (128x64 ASCII Art)

### 1. Game Intro Screen
```
┌──────────────────────────────────────────────────────────────┐
│████████████████████████████████████████████████████████████  │ 0
│██ GHOST RUNNER ███████████████████████████████████████████  │ 10
│████████████████████████████████████████████████████████████  │
│                                                               │
│             ╔═════════════════════╗                          │
│             ║  [DIFFICULTY: ★★☆]  ║                          │
│             ╚═════════════════════╝                          │ 30
│                                                               │
│                    ▶  ▶  ▶                                   │
│                                                               │
│          Match the sequence before                           │
│          time expires!                                       │ 50
│                                                               │
│             Press PRIMARY to begin                           │
│                                                               │ 64
└──────────────────────────────────────────────────────────────┘
  0                                                           128
```

**Implementation Notes:**
- Lines 0-10: Full-width inverted header (`drawBox(0,0,128,10)` + `setDrawColor(0)` + `drawStr()`)
- Lines 15-30: Bordered difficulty badge (`drawFrame()` with 2px border)
- Line 35: 16x16 play icons (`u8g2_font_open_iconic_play_2x_t`)
- Lines 40-50: Centered instruction text (6x10 font)
- Line 55: Button prompt (6x13B bold)

---

### 2. Win Screen
```
┌──────────────────────────────────────────────────────────────┐
│████████████████████████████████████████████████████████████  │ 0
│██ ★ VICTORY! ★ █████████████████████████████████████████  │ 10
│████████████████████████████████████████████████████████████  │
│                                                               │
│             ╔═══════════════════╗                            │
│             ║                   ║                            │
│             ║   SCORE: +250    ║                            │ 25
│             ║                   ║                            │
│             ╚═══════════════════╝                            │ 35
│                                                               │
│                  ★  ★  ★                                     │ 42
│                                                               │
│              MATCH COMPLETE                                  │
│                                                               │
│         [Animation hint: pulse stars]                        │
│                                                               │ 64
└──────────────────────────────────────────────────────────────┘
```

**Implementation Notes:**
- Lines 0-10: Inverted header with star glyphs (Siji font: `\ue28b` or similar)
- Lines 15-35: Thick bordered score frame (`drawFrame()` with 3px inset border)
- Line 27: Large bold score text (9x15B font)
- Line 42: Decorative star row (can animate with brightness/XOR)
- Line 48: Status message (6x10 font)

---

### 3. Lose Screen
```
┌──────────────────────────────────────────────────────────────┐
│████████████████████████████████████████████████████████████  │ 0
│██ DEFEATED ███████████████████████████████████████████████  │ 10
│████████████████████████████████████████████████████████████  │
│                                                               │
│                    ✖  ✖                                      │ 20
│                    ✖  ✖                                      │
│                                                               │
│             ╔═══════════════════╗                            │
│             ║  ATTEMPTS: 2/3   ║                            │ 32
│             ╚═══════════════════╝                            │ 38
│                                                               │
│                                                               │
│              TRY AGAIN?                                      │ 48
│          Press PRIMARY to retry                              │
│                                                               │
│         [Animation hint: flash X]                            │ 64
└──────────────────────────────────────────────────────────────┘
```

**Implementation Notes:**
- Lines 0-10: Inverted header bar
- Lines 18-24: Large X icon (16x16 from `u8g2_font_open_iconic_check_2x_t`, glyph 0x58)
- Lines 28-38: Bordered attempts counter (`drawFrame()`)
- Lines 48-54: Retry prompt (6x13B bold)

---

### 4. HUD During Gameplay (10px tall status bar)
```
┌──────────────────────────────────────────────────────────────┐
│♥♥♥ │ 1250 │ ████████░░ │ ★★☆                              │ 0-10
│──────────────────────────────────────────────────────────────│
│                                                               │
│         [Game content renders below this line]               │
│                                                               │
│                                                               │ 64
└──────────────────────────────────────────────────────────────┘
```

**Implementation Notes:**
- Line 0-10: Persistent status bar (black background via `drawBox(0,0,128,10)`)
- Segment 1 (0-20): Lives - filled heart glyphs (Siji `\u2665`)
- Segment 2 (22-50): Score - numeric text (6x10 font)
- Segment 3 (52-90): Timer - progress bar (`drawBox()` for filled, `drawFrame()` for outline)
- Segment 4 (92-128): Difficulty - star rating (Siji `\u2605` filled, `\u2606` outline)
- Use pipe separators (`│`) for visual division

---

### 5. Konami Progress Screen
```
┌──────────────────────────────────────────────────────────────┐
│████████████████████████████████████████████████████████████  │ 0
│██ CODE SEQUENCE ██████████████████████████████████████████  │ 10
│████████████████████████████████████████████████████████████  │
│                                                               │
│    ┏━━━┓ ┏━━━┓ ┏━━━┓ ┏━━━┓ ┏━━━┓ ┏━━━┓ ┏━━━┓              │ 20
│    ┃ ▲ ┃ ┃ ▲ ┃ ┃ ▼ ┃ ┃ ▼ ┃ ┃   ┃ ┃   ┃ ┃   ┃              │ 28
│    ┗━━━┛ ┗━━━┛ ┗━━━┛ ┗━━━┛ ┗━━━┛ ┗━━━┛ ┗━━━┛              │ 34
│                                                               │
│              ╔═══════════════╗                               │
│              ║               ║                               │
│              ║     4 / 7     ║                               │ 45
│              ║               ║                               │
│              ╚═══════════════╝                               │ 52
│                                                               │
│    ████████████████████████░░░░░░░░░░░░░░░░                 │ 58
│                                                               │ 64
└──────────────────────────────────────────────────────────────┘
```

**Implementation Notes:**
- Lines 0-10: Inverted header
- Lines 18-34: Seven button slots with thick borders (each 14x14 box, `drawFrame()` 2px thick)
  - Filled slots: arrow glyph inside (Siji arrows)
  - Empty slots: just the border frame
- Lines 40-52: Large progress counter (9x15B font)
- Line 58: Progress bar at bottom (filled portion + empty portion)

---

### 6. Idle/Menu Screen
```
┌──────────────────────────────────────────────────────────────┐
│████████████████████████████████████████████████████████████  │ 0
│██ HUNTER #AC7B ███████████████████████████████████████████  │ 10
│████████████████████████████████████████████████████████████  │
│                                                               │
│         ┏━━━━━━━━━━━━━━━━━━━━━━━━┓                         │
│         ┃        ╔═══╗            ┃                         │
│         ┃        ║ H ║            ┃                         │ 22
│         ┃        ╚═══╝            ┃                         │
│         ┗━━━━━━━━━━━━━━━━━━━━━━━━┛                         │ 30
│                                                               │
│     ◉ WiFi   ◉ Server   ○ Peer                              │ 36
│                                                               │
│     Wins: 12  │  Streak: 3  │  Rank: A                      │ 44
│                                                               │
│                                                               │
│        Press PRIMARY for options                             │ 58
│                                                               │ 64
└──────────────────────────────────────────────────────────────┘
```

**Implementation Notes:**
- Lines 0-10: Inverted header with player name/ID
- Lines 16-30: Bordered allegiance icon (2x scaled letter or allegiance glyph)
- Line 36: Connection status icons (Siji font: `\ue28c` filled circle, `\ue28d` outline circle)
- Line 44: Stats display with pipe separators (6x10 font)
- Line 58: Prompt text (6x13B bold)

---

## Implementation Strategy

### Phase 1: Add Drawing Primitives to Display Driver
Extend `SSD1306U8G2Driver` to expose u8g2 drawing methods:

```cpp
// Add to include/device/drivers/display.hpp
Display* drawBox(int x, int y, int w, int h) = 0;
Display* drawFrame(int x, int y, int w, int h) = 0;
Display* drawGlyph(int x, int y, uint16_t glyph) = 0;
Display* setDrawColor(int color) = 0;
Display* setFont(const uint8_t *font) = 0;

// Implement in include/device/drivers/esp32-s3/ssd1306-u8g2-driver.hpp
Display* drawBox(int x, int y, int w, int h) override {
    screen.drawBox(x, y, w, h);
    return this;
}

Display* drawFrame(int x, int y, int w, int h) override {
    screen.drawFrame(x, y, w, h);
    return this;
}

Display* drawGlyph(int x, int y, uint16_t glyph) override {
    screen.drawGlyph(x, y, glyph);
    return this;
}

Display* setDrawColor(int color) override {
    screen.setDrawColor(color);
    return this;
}

Display* setFont(const uint8_t *font) override {
    screen.setFont(font);
    return this;
}
```

### Phase 2: Create UI Helper Functions
Add to `include/game/quickdraw-resources.hpp`:

```cpp
// Bold Retro UI helpers
namespace BoldRetroUI {
    // Draw inverted header bar
    inline void drawHeaderBar(Display* display, const char* text) {
        display->setFont(u8g2_font_6x13B_tr)
               ->drawBox(0, 0, 128, 10)
               ->setDrawColor(0)
               ->drawText(text, 2, 9)
               ->setDrawColor(1);
    }

    // Draw bordered frame
    inline void drawBorderedFrame(Display* display, int x, int y, int w, int h) {
        display->drawFrame(x, y, w, h)
               ->drawFrame(x+1, y+1, w-2, h-2);
    }

    // Draw star rating
    inline void drawStarRating(Display* display, int x, int y, int filled, int total) {
        display->setFont(u8g2_font_siji_t_6x10);
        for (int i = 0; i < total; i++) {
            const char* star = (i < filled) ? "\u2605" : "\u2606";
            display->drawText(star, x + i*8, y);
        }
    }
}
```

### Phase 3: Implement Win/Lose Screens
Modify `src/game/quickdraw-states/win-state.cpp` and `lose-state.cpp` to use Bold Retro design.

---

## Font Reference

### u8g2 Bold Fonts
- `u8g2_font_6x13B_tr` - Bold 6x13 (headers)
- `u8g2_font_9x15B_tr` - Bold 9x15 (large text)
- `u8g2_font_6x10_tr` - Regular 6x10 (body)

### Icon Fonts
- `u8g2_font_siji_t_6x10` - Siji 6x10 icons (status bar elements)
  - Heart: `\u2665` (filled), `\u2661` (outline)
  - Star: `\u2605` (filled), `\u2606` (outline)
  - Circle: `\ue28c` (filled), `\ue28d` (outline)
- `u8g2_font_open_iconic_play_2x_t` - 16x16 play/media icons
- `u8g2_font_open_iconic_arrow_2x_t` - 16x16 arrows
- `u8g2_font_open_iconic_check_2x_t` - 16x16 checkmarks/X

---

## Testing Plan

1. **Simulator Testing**: Use CLI display mirror to verify layouts
2. **Hardware Testing**: Flash to ESP32-S3 and verify OLED rendering
3. **Allegiance Testing**: Test all 4 allegiances (Alleycat, Helix, Endline, Resistance)
4. **Animation Testing**: Verify LED animations sync with screen states

---

## Design Rationale

### Why Bold Retro?
1. **Readability**: Thick fonts and high contrast maximize legibility on small OLED
2. **Aesthetic Consistency**: Matches the cyberpunk LARP theme
3. **Information Density**: Siji icons pack more status data in less space
4. **Visual Impact**: Inverted headers and thick borders create strong hierarchy
5. **Retro-Futuristic**: Evokes 80s arcade/terminal aesthetic fitting for the game world

### Trade-offs vs. Current Design
**Advantages:**
- More information per screen (status bar icons)
- Clearer visual hierarchy (inverted headers)
- Better readability (bold fonts)
- Stronger thematic fit (retro aesthetic)

**Disadvantages:**
- Requires more drawing code (primitives instead of bitmaps)
- Slightly higher memory usage (more font tables)
- Less artistic flair (no custom illustrations like current win/lose images)

---

## Next Steps

1. Extend Display interface with drawing primitives
2. Implement BoldRetroUI helper namespace
3. Modify win-state.cpp and lose-state.cpp
4. Test in CLI simulator
5. Flash to hardware for final verification
6. Document learnings for future UI work

---

*Design by VM-02 | 2026-02-14*
