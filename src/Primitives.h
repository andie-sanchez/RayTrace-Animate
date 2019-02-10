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

#pragma once

#include "ofMain.h"
#include "box.h"


//  General Purpose Ray class
//  
class Ray {
public:
	glm::vec3 p, d;

	Ray(glm::vec3 p, glm::vec3 d) { 
		this->p = p; this->d = d; 
	}

	void draw(float t) { 
		ofDrawLine(p, p + t * d); 
	}

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}
};

//  ***
//  Keyframe class for Animation features
//  Holds an object's transformation channels for each keyframe
//
class Keyframe {
public:
	// position/orientation 
	//
	glm::vec3 position = glm::vec3(0, 0, 0);   // translate
	glm::vec3 rotation = glm::vec3(0, 0, 0);   // rotate
	glm::vec3 scale = glm::vec3(1, 1, 1);      // scale

	glm::vec3 pivot = glm::vec3(0, 0, 0);
	Keyframe(int fr, int fn, glm::vec3 p, glm::vec3 r, glm::vec3 s, glm::vec3 pv) {
		frame = fr;
		function = fn;
		position = p;
		rotation = r;
		scale = s;
		pivot = pv;
	}

	int frame = 0, function = 0;
	/*function types:
	0 = linear
	1 = ease in
	2 = ease out
	3 = ease in -> ease out
	4 = ease out -> ease in
	*/
};


//  Base class for any renderable object in the scene
//
class SceneObject {
public: 
	// // // VARIABLES // // //

	SceneObject *parent = NULL;        // if parent = NULL, then this obj is the ROOT
	vector<SceneObject *> childList;

	// position/orientation 
	//
	glm::vec3 position = glm::vec3(0, 0, 0);   // translate
	glm::vec3 rotation = glm::vec3(0, 0, 0);   // rotate
	glm::vec3 scale = glm::vec3(1, 1, 1);      // scale

	// rotate pivot
	//
	glm::vec3 pivot = glm::vec3(0, 0, 0);

	// material properties (we will ultimately replace this with a Material class - TBD)
	//
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;

	// UI parameters
	//
	bool isSelectable = true;
	bool isLocked = false;
	string name = "SceneObject";
	
	//  ***
	//  Keyframe animation parameters
	// 
	const static int totalFrames = 120;
	Keyframe *frames[totalFrames] = {};
	bool frmExist[totalFrames] = { false };


	// // // FUNCTIONS // // //

	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual void drawEdges() = 0;
	virtual bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal, glm::vec3 &rendCamPos) { return false; }

	// commonly used transformations
	//
	glm::mat4 getRotateMatrix() {
		return (glm::eulerAngleYXZ(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z)));   // yaw, pitch, roll 
	}

	glm::mat4 getTranslateMatrix() {
		return (glm::translate(glm::mat4(1.0), glm::vec3(position.x, position.y, position.z)));
	}

	glm::mat4 getScaleMatrix() {
		return (glm::scale(glm::mat4(1.0), glm::vec3(scale.x, scale.y, scale.z)));
	}

	glm::mat4 getLocalMatrix() {

		// get the local transformations + pivot
		//
		glm::mat4 scale = getScaleMatrix();
		glm::mat4 rotate = getRotateMatrix();
		glm::mat4 trans = getTranslateMatrix();

		// handle pivot point  (rotate around a point that is not the object's center)
		//
		glm::mat4 pre = glm::translate(glm::mat4(1.0), glm::vec3(-pivot.x, -pivot.y, -pivot.z));
		glm::mat4 post = glm::translate(glm::mat4(1.0), glm::vec3(pivot.x, pivot.y, pivot.z));
		
	    return (trans * post * rotate * pre * scale);
	}

	glm::mat4 getMatrix() {

		// if we have a parent (we are not the root),
		// concatenate parent's transform (this is recursive)
		// 
		if (parent) {
			glm::mat4 M = parent->getMatrix();
			return (M * getLocalMatrix());
		}
		else return getLocalMatrix();  // priority order is SRT
	}

	// get current Position in World Space
	//
	glm::vec3 getPosition() {
		return (getMatrix() * glm::vec4(0.0, 0.0, 0.0, 1.0));
	}

	// set position (pos is in world space)
	//
	void setPosition(glm::vec3 pos) {
		position = glm::inverse(getMatrix()) * glm::vec4(pos, 1.0);
	}

	// return a rotation  matrix that rotates one vector to another
	//
	glm::mat4 rotateToVector(glm::vec3 v1, glm::vec3 v2);

	//  Hierarchy 
	//
	void addChild(SceneObject *child) {
		childList.push_back(child);
		child->parent = this;
	}
};


// General purpose cube
//
class Cube : public SceneObject {
public:
	// // // VARIABLES // // //

	float width = 2.0;
	float height = 2.0;
	float depth = 2.0;

	// // // FUNCTIONS // // //

	Cube(glm::vec3 tran, ofColor color = ofColor::cyan) {
		position = tran;
		diffuseColor = color;
	}

	Cube(glm::vec3 tran, glm::vec3 rot, glm::vec3 sc, ofColor color = ofColor::cyan) {
		position = tran;
		rotation = rot;
		scale = sc;
		diffuseColor = color;
	}

	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal, glm::vec3 &rendCamPos);
	void draw();
	void drawEdges();
};


//  General purpose sphere  (assume parametric)
//
class Sphere: public SceneObject {
public:
	// // // VARIABLES // // //

	float radius = 1.0;

	// // // FUNCTIONS // // //

	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::magenta) { 
		position = p; 
		radius = r; 
		diffuseColor = diffuse;
	}

	Sphere(glm::vec3 p, ofColor diffuse = ofColor::magenta) {
		position = p;
		diffuseColor = diffuse;
	}
	Sphere() {}

	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal, glm::vec3 &rendCamPos);
	void draw();
	void drawEdges();
};


//  ***
//  Triangle Mesh class
//  Uses ofMesh to hold mesh properties
//
class Mesh : public SceneObject {
public:
	// // // VARIABLES // // //

	ofMesh mesh;
	vector<ofMeshFace> faces;

	// // // FUNCTIONS // // //

	Mesh(ofMesh m, glm::vec3 p = glm::vec3(0,0,0), ofColor diffuse = ofColor::yellow) {
		mesh = m;
		position = p;
		faces = mesh.getUniqueFaces();
		diffuseColor = diffuse;
	}

	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal, glm::vec3 &rendCamPos);
	void draw();
	void drawEdges();
};


//  General purpose plane 
//
class Plane: public SceneObject {
public:
	// // // VARIABLES // // //

	ofPlanePrimitive plane;
	ofMaterial material;
	glm::vec3 normal;
	float width, height;

	// // // FUNCTIONS // // //

	Plane(glm::vec3 p = glm::vec3(0,0,0), float w = 20, float h = 20, glm::vec3 n = glm::vec3(0, 1, 0), ofColor diffuse = ofColor::darkGreen) {
		position = p; 
		normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		plane.rotateDeg(-90, 1, 0, 0);
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);		
		isLocked = true;
	}

	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normalAtIntersect, glm::vec3 &rendCamPos);
	void draw();
	void drawEdges();
};


// view plane for render camera
//
class  ViewPlane : public Plane {
public:
	//  To define an infinite plane, we just need a point and normal.
	//  The ViewPlane is a finite plane so we need to define the boundaries.
	//  We will define this in terms of min, max  in 2D.  
	//  (in local 2D space of the plane)
	//  ultimately, will want to locate the ViewPlane with RenderCam anywhere
	//  in the scene, so it is easier to define the View rectangle in a local'
	//  coordinate system. 
	//

	// // // VARIABLES // // //

	glm::vec2 min, max;
	
	//  ***
	// vpersonal laptop cannot handle large objects, so view plane is cut down to this ratio 
	//
	float rt = 0.005; 

	// // // FUNCTIONS // // //

	ViewPlane(glm::vec2 p0, glm::vec2 p1) {
		min = p0;
		max = p1;
		isSelectable = false;
	}

	ViewPlane() {                         // create reasonable defaults (6x4 aspect)
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 10);
		normal = glm::vec3(0, 0, 1);      // viewplane currently limited to Z axis orientation
		isSelectable = false;
	}

	void setSize(glm::vec2 min, glm::vec2 max) {
		this->min = min;
		this->max = max;
	}

	float getAspect() {
		return width() / height();
	}

	// Convert (u, v) to (x, y, z) ; We assume u,v is in [0, 1]
	//
	glm::vec3 toWorld(float u, float v) {   //   (u, v) --> (x, y, z) [ world space ]
		float w = width();
		float h = height();
		return (glm::vec3(((u * w) + min.x)*rt, ((v * h) + min.y)*rt, position.z));
	}

	void draw() {
		ofDrawRectangle(glm::vec3(min.x*rt, min.y*rt, position.z), width()*rt, height()*rt);
	}

	//  ***
	//  draw each pixel of the view plane
	//
	void drawGrid() {
		for (float w = 0; w < width(); w++)
			for (float h = 0; h < height(); h++)
				ofDrawRectangle(glm::vec3((min.x + w)*rt, (min.y + h)*rt, position.z), rt, rt);
	}

	float width() {
		return (max.x - min.x);
	}

	float height() {
		return (max.y - min.y);
	}

	// some convenience methods for returning the corners
	//
	glm::vec2 topLeft() {
		return glm::vec2(min.x, max.y);
	}

	glm::vec2 topRight() {
		return max;
	}

	glm::vec2 bottomLeft() {
		return min;
	}

	glm::vec2 bottomRight() {
		return glm::vec2(max.x, min.y);
	}
};


//  render camera  - currently must be z axis aligned (we will improve this in project 4)
class RenderCam : public SceneObject {
public:
	// // // VARIABLES // // //

	glm::vec3 aim;
	ViewPlane view; // The camera viewplane, this is the view that we will render 

	// // // FUNCTIONS // // //

	RenderCam() {
		position = glm::vec3(0, 0, 15);
		aim = glm::vec3(0, 0, -1);
		isSelectable = false;
	}

	void setSize(glm::vec2 min, glm::vec2 max) {
		view.setSize(min, max);
	}

	Ray getRay(float u, float v) {
		// Get a ray from the current camera position to the (u, v) position on
		// the ViewPlane
		glm::vec3 pointOnPlane = view.toWorld(u, v);
		return(Ray(position, glm::normalize(pointOnPlane - position)));
	}

	void draw() {
		ofDrawBox(position, 1.0);
	};

	void drawEdges() {};

	void drawFrustum() {
		// This could be drawn a lot simpler but I wanted to use the getRay call
		// to test it at the corners.
		view.draw();
		//view.drawGrid();
		Ray r1 = getRay(0, 0); //this is in u,v
		Ray r2 = getRay(0, 1);
		Ray r3 = getRay(1, 1);
		Ray r4 = getRay(1, 0);
		float dist = glm::length((view.toWorld(0, 0) - position));
		r1.draw(dist);
		r2.draw(dist);
		r3.draw(dist);
		r4.draw(dist);
	}
};


//  ***
//  Special plane for area light calculations
//
class QuadArea : public ViewPlane {
public:
	// // // FUNCTIONS // // //

	QuadArea() {						// create reasonable defaults (6x4 aspect)
		min = glm::vec2(-2, -2);
		max = glm::vec2(2, 2);
		position = glm::vec3(0, 5, 0);
		normal = glm::vec3(0, 1, 0);	// viewplane currently limited to Z axis orientation
		isSelectable = false;

	}

	// Convert (u, v) to (x, y, z) ; We assume u,v is in [0, 1]	
	//
	glm::vec3 toWorld(float u, float v) {   //   (u, v) --> (x, y, z) [ world space ]
		float w = width();
		float h = height();
		glm::vec3 p = getPosition();
		return (glm::vec3((p.x - w / 2 + (u * w)), p.y, (p.z - h / 2 + (v * h))));
	}

	void draw() {
		plane.setPosition(position);
		plane.setWidth(width());
		plane.setHeight(height());
		plane.setResolution(2, 2);
		plane.draw();
	}
};


//  ***
//  Light class
// 
class Light : public Sphere {
public:
	// // // VARIABLES // // //

	float intensity = 10.0;
	float power = 2.0;	
	float angle = 0.5;
	glm::vec3 direction = glm::vec3(0, -1, 0);
	QuadArea area;
	int N = 20;
	int type = 0;	//0 = point light
					//1 = spot light
					//2 = area light

	// // // FUNCTIONS // // //

	Light(glm::vec3 p) {
		position = p;
		area.position = getPosition();
		area.normal = glm::vec3(0, -1, 0);
		radius = 0.5;
	}

	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normal, glm::vec3 &rendCamPos) { 
		return Sphere::intersect(ray, point, normal, rendCamPos); 
	}

	void draw() {
		if (type == 0) //ambient point light - hard shadows
			Sphere::draw();

		else if (type == 1) { //spotlight - hard shadows; limited area
			Sphere::draw();

			ofSetColor(ofColor::orange);

			glm::mat4 mInv = glm::inverse(getMatrix());			
			direction = glm::vec4(0, -1, 0, 1) * mInv;
			Ray r = Ray(getPosition(), direction);
			r.draw(5);
		}
		else if (type == 2) { //area light - soft shadows
			if (area.position != getPosition())
				area.position = getPosition();
			area.draw();
			ofSetColor(ofColor::orange);
			Sphere::draw();
		}
	}
};