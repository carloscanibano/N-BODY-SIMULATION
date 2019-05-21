#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <vector>
#include <stdint.h>
#include "SpaceObject.h"
using namespace std;

const double mass = 1000;
const double sdm = 50;
const double gravity = 6.674e-5;
const double time_interval = 0.1;
const double minimal_distance = 2.0;
const double width = 200.000;
const double height = 200.000;
const double average_mass = 1000;
const double mass_standard_deviation = 50;

auto distance_between_elements(SpaceObject n1, SpaceObject n2) {
	double first_component;
	double second_component;

	first_component = pow(n1.x - n2.x, 2);
	second_component = pow(n1.y - n2.y, 2);

	return sqrt(first_component + second_component);
}

//Only used if distance is greater than 2 units
vector<double> normal_movement(SpaceObject n1, SpaceObject n2) {
	double slope;
	double angle;
	double distance_between_n1_n2 = distance_between_elements(n1, n2);
	double n1_x_axis_force;
	double n1_y_axis_force;
	vector<double> results (2);

	//First, we have to calculate the slope
	slope = (n1.y - n2.y) / (n1.x - n2.x);

	//Check for slope correctness
	if ((slope > 1) || (slope < -1)) {
		slope = slope - ( ((int) slope) / 1);
	}

	//Slope angle calculation
	angle = atan(slope);

	//Second, we have to calculate forces
	n1_x_axis_force = ((gravity * n1.m * n2.m) / pow(distance_between_n1_n2,2));
	n1_y_axis_force = ((gravity * n1.m * n2.m) / pow(distance_between_n1_n2,2));

	if (n1_x_axis_force > 200) n1_x_axis_force = 200;
	if (n1_y_axis_force > 200) n1_y_axis_force = 200;

	n1_x_axis_force *= cos(angle);
	n1_y_axis_force *=  sin(angle);

	results[0] = n1_x_axis_force;
	results[1] = n1_y_axis_force;

	return results;
}

auto change_element_position(SpaceObject *n1, double x_axis_forces, double y_axis_forces) {
	double accel_x_axis;
	double accel_y_axis;
	double x_axis_speed;
	double y_axis_speed;
	double x_axis_new_pos;
	double y_axis_new_pos;

	accel_x_axis = x_axis_forces / n1->m;
	accel_y_axis = y_axis_forces / n1->m;

	x_axis_speed = n1->vx + (accel_x_axis * time_interval);
	y_axis_speed = n1->vy + (accel_y_axis * time_interval);

	x_axis_new_pos = n1->x + (x_axis_speed * time_interval);
	y_axis_new_pos = n1->y + (y_axis_speed * time_interval);


	n1->vx = x_axis_speed;
	n1->vy = y_axis_speed;
	n1->x = x_axis_new_pos;
	n1->y = y_axis_new_pos;
}

//Take care of edge colliding...
auto edge_handling(SpaceObject *n){
	if (n->x <= 0) {
		n->x = 2;
		n->vx = n->vx * (-1);
	}
	if (n->y <= 0) {
		n->y = 2;
		n->vy = n->vy * (-1);
	}
	if (n->x >= width) {
		n->x = width - 2;
		n->vx = n->vx * (-1);
	}
	if (n->y >= height) {
		n->y = height - 2;
		n->vy = n->vy * (-1);
	}
}

//Take care of planet colliding...
auto collision_handling(SpaceObject *n1, SpaceObject *n2) {

		//changing n1 values
		n1->vx = n2->before_vx;
		n1->vy = n2->before_vy;

		//changing n2 values
		n2->vx = n1->before_vx;
		n2->vy = n1->before_vy;
}
