# Summary
This is the set of Octave/MATLAB scripts which represents interpolation algorithm implemented inside RozumRobotics servo drives.
Interpolation algorithm uses so called PVAT points (Postion Velocity Acceleration Time) to build smooth trajectory with. 
### Content
`qupsample.m` - function to interpolate trajectory descibed by the set of PVAT points  
`trj.m` - function to interpolate a piece of trajectory between two points  
`trj_coeff.m` - function to calculate a set of coefficients for a two given points  
`ptcat.m` - function to concatenate two trajectories described by two point sets  
`move_to.m` - function to generate trajectory to move from start position to final position with velocity and acceleration constraints  
`trj_plot.m` - example program which demonstrates the use of previously mentioned functions  
### Run
Just run `trj_plot.m` script using Octave or MATLAB. The figure with sample generated trajectory should be displayed.
