#ifndef SpaceObject_h
#define SpaceObject_h
#include <vector>
#include <iostream>
using namespace std;

class SpaceObject{
public:
	//X axis position
	double x;
	//Y axis position
	double y;
	//X axis speed
	double vx;
	//Y axis speed
	double vy;
	//Before collision X axis speed
	double before_vx;
	//Before collision Y axis speed
	double before_vy;
	//Before change x position
	double before_x;
	//Before change y position
	double before_y;
	//Object mass
	double m;
	//Who collides with this asteroid
	int collisions;
	//Default constructor
	SpaceObject(double, double, double, double, double);
	void set_values(double, double, double, double, double);
};

SpaceObject::SpaceObject(double x1, double y1, double vx1, double vy1, double m1){
	x = x1;
	y = y1;
	vx = vx1;
	vy = vy1;
	m = m1;
	//Collision counter
	collisions = 0;
	before_vx = 0;
	before_vy = 0;
	before_x = 0;
	before_y = 0;
}
#endif
