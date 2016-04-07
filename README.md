# OpenCV-Panorama
Sample OpenCV project with c++

Reads multiple FishEye lense photographs and outputs 360 panorama image
Spherical coordinates projected onto polar coordinates and images are blended together.

Current Features:
- Simple projection transformation as proof of concept

Future versions:
- Use OpenCV projection framework to store, load and apply the projection
- use BI_LINEAR interpolation and anti-aliazing for higher quality
- use hardware acceleration to improve performance
- obtain more sample data to improve blending quality
- use feature detection to improve homography transformations
