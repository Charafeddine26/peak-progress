"""
Peak Progress — Climber Figure
Tool: Blender (Python script via Scripting tab)

HOW TO USE:
  1. Open Blender (free): https://www.blender.org/download/
  2. At the top, click the "Scripting" tab
  3. Click "Open" and load this file  (or paste it into the text editor)
  4. Click "Run Script" (the play button ▶)
  5. The climber figure will appear in the viewport
  6. File > Export > Stl (.stl)
  7. Make sure "Selection Only" is checked, filename = climber_blender.stl
  8. Send the STL to your slicer (Cura / PrusaSlicer)

Print settings: PLA, 0.2mm layer height, 20% infill, no supports needed
"""

import bpy
import bmesh
import math

# ============================================================
# Parameters (edit these to resize)
# ============================================================
TOTAL_HEIGHT     = 0.040   # m  — 40mm total figure height
BASE_RADIUS      = 0.006   # m  — foot disc radius
BASE_HEIGHT      = 0.003   # m  — foot disc thickness
BODY_RADIUS      = 0.0045  # m  — torso radius
BODY_HEIGHT      = 0.014   # m  — torso height
NECK_HEIGHT      = 0.004   # m  — neck height
NECK_RADIUS      = 0.0025  # m  — neck radius
HEAD_RADIUS      = 0.005   # m  — head sphere radius
HOLE_DIAMETER    = 0.0016  # m  — fishing line hole diameter
HOLE_DEPTH       = 0.006   # m  — fishing line hole depth
SEGMENTS         = 32      # smoothness

# ============================================================
# Helpers
# ============================================================

def deselect_all():
    bpy.ops.object.select_all(action='DESELECT')

def apply_all_modifiers(obj):
    bpy.context.view_layer.objects.active = obj
    for mod in obj.modifiers:
        bpy.ops.object.modifier_apply(modifier=mod.name)

def join_objects(objects):
    deselect_all()
    for o in objects:
        o.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    return bpy.context.active_object

# ============================================================
# Clean up any previous "Climber" objects
# ============================================================
for obj in bpy.data.objects:
    if obj.name.startswith("Climber"):
        bpy.data.objects.remove(obj, do_unlink=True)

# ============================================================
# Build the figure
# ============================================================
parts = []

# --- Base disc ---
bpy.ops.mesh.primitive_cylinder_add(
    radius=BASE_RADIUS,
    depth=BASE_HEIGHT,
    vertices=SEGMENTS,
    location=(0, 0, BASE_HEIGHT / 2)
)
base = bpy.context.active_object
base.name = "Climber_base"
parts.append(base)

# --- Body (slightly tapered cylinder) ---
body_z = BASE_HEIGHT + BODY_HEIGHT / 2
bpy.ops.mesh.primitive_cone_add(
    radius1=BODY_RADIUS + 0.001,
    radius2=BODY_RADIUS,
    depth=BODY_HEIGHT,
    vertices=SEGMENTS,
    location=(0, 0, BASE_HEIGHT + BODY_HEIGHT / 2)
)
body = bpy.context.active_object
body.name = "Climber_body"
parts.append(body)

# --- Neck (tapered) ---
neck_z = BASE_HEIGHT + BODY_HEIGHT + NECK_HEIGHT / 2
bpy.ops.mesh.primitive_cone_add(
    radius1=BODY_RADIUS,
    radius2=NECK_RADIUS,
    depth=NECK_HEIGHT,
    vertices=SEGMENTS,
    location=(0, 0, BASE_HEIGHT + BODY_HEIGHT + NECK_HEIGHT / 2)
)
neck = bpy.context.active_object
neck.name = "Climber_neck"
parts.append(neck)

# --- Head (sphere) ---
head_z = BASE_HEIGHT + BODY_HEIGHT + NECK_HEIGHT + HEAD_RADIUS * 0.7
bpy.ops.mesh.primitive_uv_sphere_add(
    radius=HEAD_RADIUS,
    segments=SEGMENTS,
    ring_count=SEGMENTS // 2,
    location=(0, 0, head_z)
)
head = bpy.context.active_object
head.name = "Climber_head"
# Smooth shading on head
bpy.ops.object.shade_smooth()
parts.append(head)

# --- Left arm bump ---
arm_z = BASE_HEIGHT + BODY_HEIGHT - 0.002
bpy.ops.mesh.primitive_uv_sphere_add(
    radius=0.0022,
    segments=16,
    ring_count=8,
    location=(BODY_RADIUS + 0.001, 0, arm_z)
)
arm_l = bpy.context.active_object
arm_l.name = "Climber_arm_l"
parts.append(arm_l)

# --- Right arm bump ---
bpy.ops.mesh.primitive_uv_sphere_add(
    radius=0.0022,
    segments=16,
    ring_count=8,
    location=(-(BODY_RADIUS + 0.001), 0, arm_z)
)
arm_r = bpy.context.active_object
arm_r.name = "Climber_arm_r"
parts.append(arm_r)

# ============================================================
# Join all parts into one mesh
# ============================================================
climber = join_objects(parts)
climber.name = "Climber"

# ============================================================
# Drill fishing line hole at top of head using Boolean
# ============================================================
hole_z = head_z + HEAD_RADIUS - HOLE_DEPTH / 2 + 0.001
bpy.ops.mesh.primitive_cylinder_add(
    radius=HOLE_DIAMETER / 2,
    depth=HOLE_DEPTH + 0.002,
    vertices=16,
    location=(0, 0, hole_z)
)
hole_cutter = bpy.context.active_object
hole_cutter.name = "Climber_hole_cutter"

# Apply boolean difference
deselect_all()
climber.select_set(True)
bpy.context.view_layer.objects.active = climber

bool_mod = climber.modifiers.new(name="FishingLineHole", type='BOOLEAN')
bool_mod.operation = 'DIFFERENCE'
bool_mod.object = hole_cutter
bool_mod.solver = 'FAST'

apply_all_modifiers(climber)

# Remove cutter
bpy.data.objects.remove(hole_cutter, do_unlink=True)

# ============================================================
# Final cleanup
# ============================================================
deselect_all()
climber.select_set(True)
bpy.context.view_layer.objects.active = climber

print("✓ Climber figure created successfully.")
print(f"  Approximate height: {TOTAL_HEIGHT * 1000:.0f}mm")
print(f"  Fishing line hole: {HOLE_DIAMETER * 1000:.1f}mm diameter at top")
print("")
print("Next: File > Export > Stl (.stl)  →  check 'Selection Only'  →  Save")
