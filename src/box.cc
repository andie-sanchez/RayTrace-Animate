//
//   Andie Sanchez
//   2 February 2019


//   ALL ORIGINAL CLASSES & FUNCTIONS WILL BE MARKED with " *** "

#include "vector3.h"
#include "ray.h"
#include "box.h"
  
/*
 * Ray-box intersection using IEEE numerical properties to ensure that the
 * test is both robust and efficient, as described in:
 *
 *      Amy Williams, Steve Barrus, R. Keith Morley, and Peter Shirley
 *      "An Efficient and Robust Ray-Box Intersection Algorithm"
 *      Journal of graphics tools, 10(1):49-54, 2005
 *
 */

bool Box::intersect(const _Ray &r, Vector3 &point, Vector3 &normal, float t0, float t1) const {
	float tmin, tmax, txmin, txmax, tymin, tymax, tzmin, tzmax;

	tmin = txmin = (parameters[r.sign[0]].x() - r.origin.x()) * r.inv_direction.x();
	tmax = txmax = (parameters[1-r.sign[0]].x() - r.origin.x()) * r.inv_direction.x();
	tymin = (parameters[r.sign[1]].y() - r.origin.y()) * r.inv_direction.y();
	tymax = (parameters[1-r.sign[1]].y() - r.origin.y()) * r.inv_direction.y();
	if ( (tmin > tymax) || (tymin > tmax) ) 
	return false;
	if (tymin > tmin)
	tmin = tymin;
	if (tymax < tmax)
	tmax = tymax;
	tzmin = (parameters[r.sign[2]].z() - r.origin.z()) * r.inv_direction.z();
	tzmax = (parameters[1-r.sign[2]].z() - r.origin.z()) * r.inv_direction.z();
	if ( (tmin > tzmax) || (tzmin > tmax) ) 
	return false;
	if (tzmin > tmin)
	tmin = tzmin;
	if (tzmax < tmax)
	tmax = tzmax;
	bool hit = ( (tmin < t1) && (tmax > t0) );

	//  ***
	//  calculate the POI and the face normal
	//
	if (hit) {
		float x = r.origin[0] + r.direction[0] * tmin;
		float y = r.origin[1] + r.direction[1] * tmin;
		float z = r.origin[2] + r.direction[2] * tmin;
		point = Vector3(x, y, z);

		//determing face normal referenced from
		//http://ray-tracing-conept.blogspot.com/2015/01/ray-box-intersection-and-normal.html
		//
		float EPSI = 0.01;
		if (fabs(point[0] - parameters[0][0]) < EPSI)
			normal = Vector3(-1, 0, 0);
		else if (fabs(point[0] - parameters[1][0]) < EPSI)
			normal = Vector3(1, 0, 0);
		else if (fabs(point[1] - parameters[0][1]) < EPSI)
			normal = Vector3(0, -1, 0);
		else if (fabs(point[1] - parameters[1][1]) < EPSI)
			normal = Vector3(0, 1, 0);
		else if (fabs(point[2] - parameters[0][2]) < EPSI)
			normal = Vector3(0, 0, -1);
		else if (fabs(point[2] - parameters[1][2]) < EPSI)
			normal = Vector3(0, 0, 1);
	}

	return hit;
}