//
//   Andie Sanchez
//   2 February 2019


//
//  Primitives.h - Simple 3D Primitives with with Hierarchical Transformations
//
//  
//  (c) Kevin M. Smith  - 24 September 2018
//

//  THIS PROJECT USES SKELETON CODE PROVIDED IN CS 116A 
//  ALL ORIGINAL CLASSES & FUNCTIONS WILL BE MARKED with " *** "


#include "ofApp.h"
#include "Primitives.h"


// // // INTERSECTION FUNCTIONS // // //


bool Sphere::intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal, glm::vec3 &rendCamPos) {

	// transform Ray to object space.  
	//
	glm::mat4 m = getMatrix();
	glm::mat4 mInv = glm::inverse(m);
	glm::vec4 p = mInv * glm::vec4(ray.p.x, ray.p.y, ray.p.z, 1.0);
	glm::vec4 p1 = mInv * glm::vec4(ray.p + ray.d, 1.0);
	glm::vec3 d = glm::normalize(p1 - p);

	bool hit = glm::intersectRaySphere(glm::vec3(p), d, glm::vec3(0, 0, 0), radius, point, normal);
	
	// ***	
	// transform point and normal to world space
	//
	if (hit) {
		point = m * glm::vec4(point, 1.0);
		normal = glm::vec4(normal, 1.0) * mInv;
	}

	return hit;
}


//  Cube::intersect - test intersection with the unit Cube.  Note that
//  intersection test is done in object space with an axis aligned box (AAB), 
//  the input ray is provided in world space, so we need to transform the ray to object space.
//  this method does NOT return a normal.
//
bool Cube::intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal, glm::vec3 &rendCamPos) {

	// transform Ray to object space.  
	//
	glm::mat4 m = getMatrix();
	glm::mat4 mInv = glm::inverse(m);
	glm::vec4 p = mInv * glm::vec4(ray.p.x, ray.p.y, ray.p.z, 1.0);
	glm::vec4 p1 = mInv * glm::vec4(ray.p + ray.d, 1.0);
	glm::vec3 d = glm::normalize(p1 - p);


	// intesect method we use will be Willam's  (see box.h and box.cc for reference).
	// note that this class has it's own version of Ray, Vector3  (TBD: port to GLM)
	//
	_Ray boxRay = _Ray(Vector3(p.x, p.y, p.z), Vector3(d.x, d.y, d.z));

	// we will test for intersection in object space (object is a "unit" cube edge is len=2)
	//
	Box box = Box(Vector3(-width/2.0, -height/2.0, -depth/2.0), Vector3(width/2.0, height/2.0, depth/2.0));

	Vector3 pt, nm;

	//calculates the POI & normal
	bool hit = box.intersect(boxRay, pt, nm, -1000, 1000);

	// ***
	// transform point and normal to world space
	//
	if (hit) {		
		point = glm::vec3(pt[0], pt[1], pt[2]);
		point = m * glm::vec4(point, 1.0);
		normal = glm::vec4(nm[0], nm[1], nm[2], 1.0) * mInv;
	}

	return hit;
}


// ***
// Determines if ray intersects mesh
// Returns point of intersection and face normal
// Currently does not support smooth shading
//
bool Mesh::intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal, glm::vec3 &rendCamPos) {
	
	// transform Ray to object space.  
	//
	glm::mat4 m = getMatrix();
	glm::mat4 mInv = glm::inverse(m);
	glm::vec4 p0 = mInv * glm::vec4(ray.p.x, ray.p.y, ray.p.z, 1.0);
	glm::vec4 p1 = mInv * glm::vec4(ray.p + ray.d, 1.0);
	glm::vec3 p = p0; //DO NOT NORMALIZE
	glm::vec3 d = glm::normalize(p1 - p0);
	glm::vec3 r = mInv * glm::vec4(rendCamPos, 1.0);

	bool anyHit = false, hit;
	float near_dist = std::numeric_limits<float>::infinity(), dist, front;
	glm::vec3 bary, pt, nm, v0, v1, v2;

	for (int i = 0; i < faces.size(); i++) {
		hit = false;
		v0 = faces[i].getVertex(0);
		v1 = faces[i].getVertex(1);
		v2 = faces[i].getVertex(2);
		nm = faces[i].getFaceNormal();

		hit = glm::intersectRayTriangle(p, d, v0, v1, v2, bary);
		front = glm::dot(d, nm);

		if (hit && front < 0) {
			pt = p + bary.z * d;
			dist = glm::length(pt - r);
			if (dist < near_dist) {
				near_dist = dist;
				point = pt;
				normal = faces[i].getFaceNormal();

				//calculate the interpolated normal for shading
				
			}
			anyHit = true;
		}
	}

	if (anyHit) {
		//convert to world space
		point = m * glm::vec4(point, 1.0);
		normal = glm::vec4(normal, 1.0) * mInv;
	}

	return anyHit;
}


// Intersect Ray with Plane  (wrapper on glm::intersect*)
//
bool Plane::intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normalAtIntersect, glm::vec3 &rendCamPos) {
	float dist;
	bool insidePlane = false;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = this->normal;
		glm::vec2 xrange = glm::vec2(position.x - width / 2, position.x + width / 2);
		glm::vec2 zrange = glm::vec2(position.z - height / 2, position.z + height / 2);
		if (point.x < xrange[1] && point.x > xrange[0] && point.z < zrange[1] && point.z > zrange[0]) {
			insidePlane = true;
		}
	}
	return insidePlane;
}


// // // DRAWING FUNCTIONS // // //


void Sphere::draw() {

	//   get the current transformation matrix for this object
	//
	glm::mat4 m = getMatrix();

	//   push the current stack matrix and multiply by this object's
	//   matrix. now all vertices dran will be transformed by this matrix
	//
	ofPushMatrix();
	ofMultMatrix(m);
	ofDrawSphere(radius);
	ofPopMatrix();

	// draw axis
	//
	ofApp::drawAxis(m, 1.5);
	
}


// Draw a Unit cube (size = 2) transformed 
//
void Cube::draw() {

    //   get the current transformation matrix for this object
	//
	glm::mat4 m = getMatrix();

	//   push the current stack matrix and multiply by this object's
	//   matrix. now all vertices dran will be transformed by this matrix
	//
	ofPushMatrix();
	ofMultMatrix(m);
	ofDrawBox(width, height, depth);
	ofPopMatrix();

	// draw axis
	//
	ofApp::drawAxis(m, 1.5);

}


//  ***
//  draws triangle mesh using ofMesh draw
//
void Mesh::draw() {
	//  get the current transformation matrix for this object
	//
	glm::mat4 m = getMatrix();

	//  push the current stack matrix and multiply by this object's
	//  matrix. now all vertices dran will be transformed by this matrix
	//
	ofPushMatrix();
	ofMultMatrix(m);
	mesh.draw();
	ofPopMatrix();

	//  draw axis
	//
	ofApp::drawAxis(m, 1.5);
}


//  Uses OF plane primitive
//
void Plane::draw() {
	material.begin();
	material.setDiffuseColor(diffuseColor);
	plane.setWidth(width);
	plane.setHeight(height);
	plane.setResolution(5, 5);
	plane.draw();
	material.end();
}


// // // EDGE ONLY DRAWING FUNCTIONS // // //


//  ***
void Sphere::drawEdges() {

	//  get the current transformation matrix for this object
	//
	glm::mat4 m = getMatrix();

	//  push the current stack matrix and multiply by this object's
	//  matrix. now all vertices dran will be transformed by this matrix
	//
	ofNoFill();
	ofPushMatrix();
	ofMultMatrix(m);
	ofDrawSphere(radius);
	ofPopMatrix();
	ofFill();
}


//  ***
//  Draw a Unit cube (size = 2) transformed 
//
void Cube::drawEdges() {

	//  get the current transformation matrix for this object
	//
	glm::mat4 m = getMatrix();

	//  push the current stack matrix and multiply by this object's
	//  matrix. now all vertices dran will be transformed by this matrix
	//
	ofNoFill();
	ofPushMatrix();
	ofMultMatrix(m);
	ofDrawBox(width, height, depth);
	ofPopMatrix();
	ofFill();
}


//  ***
void Mesh::drawEdges() {
	//  mesh.drawFaces();
	//  get the current transformation matrix for this object
	//
	glm::mat4 m = getMatrix();

	//  push the current stack matrix and multiply by this object's
	//  matrix. now all vertices dran will be transformed by this matrix
	//
	ofPushMatrix();
	ofMultMatrix(m);
	mesh.drawWireframe();
	ofPopMatrix();

	//  draw axis
	//
	ofApp::drawAxis(m, 1.5);
}


//  ***
void Plane::drawEdges() {
	material.begin();
	plane.setWidth(width);
	plane.setHeight(height);
	plane.setResolution(5, 5);
	plane.drawWireframe();
	material.end();
}


// // // SCENE OBJECT FUNCTION // // //


// Generate a rotation matrix that rotates v1 to v2
// v1, v2 must be normalized
//
glm::mat4 SceneObject::rotateToVector(glm::vec3 v1, glm::vec3 v2) {

	glm::vec3 axis = glm::cross(v1, v2);
	glm::quat q = glm::angleAxis(glm::angle(v1, v2), glm::normalize(axis));
	return glm::toMat4(q);
}