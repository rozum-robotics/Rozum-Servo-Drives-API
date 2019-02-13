/**
 * @brief Checking PVT points 
 * @file check_motion_points.c
 * @author Rozum
 * @date 2018-12-05
 */

#include "api.h"

/**
 * Maximum velocities (degrees per seconds) for the RDrive motors (@48V):
 *  - RD50: 267.84 ~= 260
 *  - RD60: 267.84 ~= 260
 *  - RD85: 244.80 ~= 240
 */

float velocity_limit = 80.0;

int main(int argc, char *argv[])
{
    typedef struct
    {
        float pos;
        float vel;
        float time;
    } point_t;

    point_t points[] = {
        {343.77, 0, 0},
        {202.57, 1.33, 1537.4},
        {205.63, 9.20, 1148.9},
        {210.00, 21.24, 277.9},
        {215.61, 32.27, 209.5},
        {222.35, 42.94, 178.5},
        {230.14, 52.62, 161.8},
        {238.85, 60.75, 152.6},
        {248.41, 67.20, 148.5},
        {258.72, 72.29, 147.1},
        {269.68, 76.51, 147.1},
        {281.23, 79.88, 147.1},
        {293.29, 81.69, 148.4},
        {305.8, 81.01, 152.4},
        {318.72, 77.22, 161.6},
        {331.98, 69.83, 178.0},
        {345.56, 57.72, 208.5},
        {359.42, 35.09, 275.4},
        {373.52, 0.00, 709.8}};

    for(uint32_t i = 0; i < sizeof(points) / sizeof(points[0]) - 1; i++)
    {
        float calc_velocity;
        bool over_speed = rr_check_point(velocity_limit, &calc_velocity,
                                         points[i].pos, points[i].vel,
                                         points[i + 1].pos, points[i + 1].vel,
                                         points[i + 1].time);

        API_DEBUG("Point Angles (Velocities): %.3f (%.3f) -> %.3f (%.3f) [%d ms] # Limit Vel: %.3f | Max Vel: %.3f %s\n",
                  points[i].pos, points[i].vel,
                  points[i + 1].pos, points[i + 1].vel,
                  (int)points[i + 1].time,
                  velocity_limit, calc_velocity,
                  over_speed ? "is too high" : "");
    }
}