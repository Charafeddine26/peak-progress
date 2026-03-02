// ============================================================
// Peak Progress — Climber Figure
// Tool: OpenSCAD
// ============================================================
// HOW TO USE:
//   1. Install OpenSCAD (free): https://openscad.org/downloads.html
//   2. Open this file in OpenSCAD
//   3. Press F6 to render (takes a few seconds)
//   4. File > Export > Export as STL
//   5. Send the STL to your slicer (Cura / PrusaSlicer)
//
// Print settings: PLA, 0.2mm layer height, 20% infill, no supports needed
// ============================================================

// --- Parameters (edit these to resize) ---
total_height     = 40;   // mm — total figure height
base_radius      = 6;    // mm — foot disc radius
base_height      = 3;    // mm — foot disc thickness
body_radius      = 4.5;  // mm — torso cylinder radius
body_height      = 14;   // mm — torso cylinder height
neck_radius      = 2.5;  // mm — neck taper radius
neck_height      = 4;    // mm — neck height
head_radius      = 5;    // mm — head sphere radius
hole_diameter    = 1.6;  // mm — fishing line hole at top
hole_depth       = 6;    // mm — how deep the hole goes into the head
fn               = 64;   // smoothness (higher = smoother, slower render)

// --- Derived values ---
body_z    = base_height;
neck_z    = body_z + body_height;
head_z    = neck_z + neck_height + head_radius * 0.6;

// ============================================================
// Model
// ============================================================
difference() {
    union() {

        // Base disc (feet)
        cylinder(h = base_height, r = base_radius, $fn = fn);

        // Body — tapered from base_radius to body_radius
        translate([0, 0, body_z])
            cylinder(h = body_height, r1 = body_radius + 1, r2 = body_radius, $fn = fn);

        // Neck — tapered from body_radius down to neck_radius
        translate([0, 0, neck_z])
            cylinder(h = neck_height, r1 = body_radius, r2 = neck_radius, $fn = fn);

        // Head — sphere
        translate([0, 0, head_z])
            sphere(r = head_radius, $fn = fn);

        // Small shoulder bumps (arms suggestion)
        translate([ body_radius + 1, 0, neck_z - 2])
            sphere(r = 2.2, $fn = fn);
        translate([-body_radius - 1, 0, neck_z - 2])
            sphere(r = 2.2, $fn = fn);
    }

    // Fishing line hole — drilled straight down from the top of the head
    translate([0, 0, head_z + head_radius - hole_depth + 0.1])
        cylinder(h = hole_depth, d = hole_diameter, $fn = 32);
}
