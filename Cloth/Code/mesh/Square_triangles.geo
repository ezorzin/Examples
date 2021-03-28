//////////////////////////////////////////////////////////////////
//
//      D---------C
//      |         |
//      |         |
//      |         |
//      A---------B
//
//
//      y
//      |
//      |
//      o -----x
//     /
//    /
//   z
//
//////////////////////////////////////////////////////////////////

ds = 0.2;                                                       // Setting side discretization length...
x_min = -1.0;                                                   // Setting "x_min"...
x_max = +1.0;                                                   // Setting "x_max"...
y_min = -1.0;                                                   // Setting "y_min"...
y_max = +1.0;                                                   // Setting "y_max"...

Point(1) = {x_min, y_min, 0.0, ds};                             // Setting point "A"...
Point(2) = {x_max, y_min, 0.0, ds};                             // Setting point "B"...
Point(3) = {x_max, y_max, 0.0, ds};                             // Setting point "C"...
Point(4) = {x_min, y_max, 0.0, ds};                             // Setting point "D"...

Line(10) = {1, 2};                                              // Setting side "AB"...
Line(20) = {2, 3};                                              // Setting side "BC"...
Line(30) = {3, 4};                                              // Setting side "CD"...
Line(40) = {4, 1};                                              // Setting side "AD"...

Curve Loop(50) = {10, 20, 30, 40};                              // Setting perimeter "ABCD"...

Plane Surface(100) = {50};                                      // Setting surface "ABCD"...

Physical Curve(60) = {10, 20, 30, 40};                          // Setting group: perimeter "ABCD"...
Physical Curve(70) = {10};                                      // Setting group: side "AB"...
Physical Curve(80) = {40};                                      // Setting group: side "AD"...
Physical Surface(200) = {100};                                  // Setting group: surface "ABCD"...

Mesh 2;                                                         // Setting mesh type: triangles...

Mesh.SaveAll = 1;                                               // Saving all mesh nodes...