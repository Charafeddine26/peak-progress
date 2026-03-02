# Climber Figure — Blender

## Setup (one time)
1. Download and install **Blender** (free): https://www.blender.org/download/

## Generate the STL
1. Open Blender
2. At the top bar, click the **Scripting** tab
3. Click **Open** → select `climber_blender.py`
4. Click **Run Script** (▶ button, top right of the script editor)
5. The "Climber" object will appear in the viewport
6. **File → Export → Stl (.stl)**
   - Check **Selection Only**
   - Filename: `climber_blender.stl`
   - Click **Export STL**

## Send to printer
Open the STL in your slicer (Cura / PrusaSlicer):
- Material: PLA
- Layer height: 0.2mm
- Infill: 20%
- Supports: **none needed**
- Estimated print time: ~30–40 min

## Tweaking dimensions
All parameters are at the top of `climber_blender.py` under the `Parameters` section.
Edit the values (they are in meters — so 0.040 = 40mm), then re-run the script.
