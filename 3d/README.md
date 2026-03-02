# Peak Progress — 3D Printing

This directory contains all 3D models for the Peak Progress device.

## Benchmark: Climber Figure

**Goal:** Model the same piece — the climber figure — in both OpenSCAD and Blender,
compare results, and decide which tool the team will use going forward.

### The Piece
- Pawn-shaped humanoid figure (~40mm tall)
- Fishing line hole at top (1.5mm diameter)
- Mounts to servo arm, rides up the mountain face

### Folders

| Folder | Tool | Approach |
|---|---|---|
| `benchmark/climber-openscad/` | OpenSCAD | Code-based parametric model — I write it, you render + export |
| `benchmark/climber-blender/` | Blender | Python script — paste in Scripting tab, run, export STL |

### Which to send to the printer?
After both are generated, open each STL in your slicer (Cura / PrusaSlicer).
Whichever looks cleaner and has no mesh errors → use that one.
The Blender version will likely look more organic. The OpenSCAD version will be more precise.

---

## Final Parts (to be added after benchmark)

| Part | File | Notes |
|---|---|---|
| Mountain shell | `mountain/mountain_shell.scad` | 250mm tall, splits into halves |
| Servo bracket | `parts/servo_bracket.scad` | Fits SG90, mounts inside upper cavity |
| Peak pulley | `parts/pulley.scad` | Redirects fishing line at summit |
| Base tray | `parts/base_tray.scad` | Houses Arduino + Circuit Playground |
| Summit flag | `parts/summit_flag.scad` | Swappable, LED-illuminated |
| Climber figure | `benchmark/` winner | From benchmark result |
