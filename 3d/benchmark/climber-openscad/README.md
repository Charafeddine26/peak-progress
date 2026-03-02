# Climber Figure — OpenSCAD

## Setup (one time)
1. Download and install **OpenSCAD** (free): https://openscad.org/downloads.html

## Generate the STL
1. Open `climber.scad` in OpenSCAD
2. Press **F6** to render (wait ~5 seconds)
3. **File → Export → Export as STL**
4. Save as `climber_openscad.stl`

## Send to printer
Open the STL in your slicer (Cura / PrusaSlicer):
- Material: PLA
- Layer height: 0.2mm
- Infill: 20%
- Supports: **none needed**
- Estimated print time: ~30–40 min

## Tweaking dimensions
All parameters are at the top of `climber.scad` — edit the numbers, press F6 again.
