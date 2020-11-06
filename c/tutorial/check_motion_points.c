/**
 * @brief Checking PVT points 
 * @file check_motion_points.c
 * @author Rozum
 * @date 2018-12-05
 */

#include "api.h"

/**
 *  \defgroup tutor_c_check_motion_points Calibrating to mitigate cogging effects
 *  The tutorial demonstrates how to verify validity of PVT points in a servo trajectory
 *  without actually connecting a servo, while checking for PVT velocities over maximum limits.
 *  Maximum velocity limits (degrees per seconds) for RDrive motors (@48V):
 *  <ul><li>RD50: 267.84 ~= 260</li>
 *  <li>RD60: 267.84 ~= 260</li>
 *  <li> RD85: 244.80 ~= 240</li></ul>
 *
 * 1. Create an array of PVT points to define the desired motion trajectory.
 * \snippet check_motion_points.c Create PVT
 * 2. Check the PVT points for velocities over the maximum limit.
 * \snippet check_motion_points.c Check velocity at PVT
 * Finally, the function displays calculation results for each of the PVT points.
 *
 *<b> Complete tutorial code: </b>
 * \snippet check_motion_points.c check_motion_points_code_full
 */

//! [check_motion_points_code_full]
float velocity_limit = 80.0;

//! [Create PVT]
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
	//! [Create PVT]

	//! [Check velocity at PVT]
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
	//! [Check velocity at PVT]
}
//! [check_motion_points_code_full]
