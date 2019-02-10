//
//   Andie Sanchez
//   2 February 2019


//   ALL ORIGINAL CLASSES & FUNCTIONS WILL BE MARKED with " *** "


//
//  Includes code from starter files for Projects 2 (RayTracer) & 3 (Skeleton Builder)
//
//  This file includes scene setup and switching camera views.
//
//  This file includes functionality that supports selection and translate/rotation
//  of scene objects using the mouse.
//
//  Modifer keys for rotatation are x, y and z keys (for each axis of rotation)
//
//  (c) Kevin M. Smith  - 24 September 2018
//

//  THIS PROJECT USES SKELETON CODE PROVIDED IN CS 116A 
//  ALL ORIGINAL CLASSES & FUNCTIONS WILL BE MARKED with " *** "

//  project currently has no pivot support

#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxAssimpModelLoader.h"
#include "Primitives.h"

class ofApp : public ofBaseApp{

	public:
		// // // DEFAULT OF FUNCTIONS // // //
		// defined in ofApp.cpp
		//
		void setup();	// ***
		void update();  // ***
		void draw();	// ***

		void keyPressed(int key);	// ***
		void keyReleased(int key);	// ***
		
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void dragEvent(ofDragInfo dragInfo);

		// CS116A skeleton functions
		//
		bool mouseToDragPlane(int x, int y, glm::vec3 &point);
		bool objSelected() { return (selected.size() ? true : false); };

		// unused default functions
		//
		void mouseMoved(int x, int y );
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);		
		void gotMessage(ofMessage msg);
		

		// // // RAYTRACING FUNCTIONS // // //
		// ***
		// defined in raytrace.cpp
		//
		void raytrace();
		ofColor lambert(const glm::vec3 &p, const glm::vec3 &norm, 
						int i, const ofColor diffuse);
		ofColor phong(const glm::vec3 &v, const glm::vec3 &norm, 
						int i,	const ofColor diffuse, const ofColor specular);


		// // // ANIMATION FUNCTIONS // // //
		// ***
		// defined in animate.cpp
		//
		void addKeyframe();
		void delKeyframe();
		void updateFrame();
		void advanceFrame();
		void calcFrame(int i, int frame);
		

		// // // SCENE FUNCTIONS // // //
		// ***
		// defined in scene.cpp
		//
		void delObj();						// delete selected object
		void newObj(char key);				// create new primitive object
		void setFrmSldColor(bool exists);	// set color of frame slider
		void updateSliders();				// update slider values
		void updateSelected(bool newFrm);	// update selected object's values

		// CS116A skeleton functions
		//
		static void drawAxis(glm::mat4 transform = glm::mat4(1.0), float len = 1.0);
		void printChannels(SceneObject *);


		// // // GLOBAL VARIABLES // // //

		// skeleton code variables
		//

		// Cameras
		//
		ofEasyCam  mainCam;
		ofCamera sideCam;
		ofCamera topCam;
		ofCamera rndrCam;
		ofCamera  *theCam;    // set to current camera either mainCam or sideCam

		// Materials
		//
		ofMaterial material;

		// scene components
		//
		vector<SceneObject *> scene;
		vector<SceneObject *> selected;
		SceneObject *prevSelected = NULL;
		glm::vec3 lastPoint;
		vector<Light*> lights;		// ***		
		ofxAssimpModelLoader model; // ***		

		// set up one render camera to render image through
		//
		RenderCam renderCam;

		// ***
		// original variables
		//
		
		// for raytracing
		//
		ofImage image;
		float imgW, imgH;
		ofColor ambientColor = ofColor(100, 100, 100);
		ofColor bkgndColor = ofColor::black;

		// for animation
		//
		int numObj = 1;
		int currFrm = 0;
		int totalFrames = SceneObject::totalFrames;
		int frmCnt, foldCnt;
		vector<int> recentKF, nextKF;

		// for gui
		//
		ofxPanel gui, animGui;
		ofxIntSlider typeSld, NSld, rSld, gSld, bSld, frmSld, fnSld;
		ofxFloatSlider pwrSld, intstSld, splDegSld, widthSld, heightSld;		

		// States
		//
		bool bDrag = false;		// mouse drag
		bool bImage = true;		// show render output
		bool bHide = false;		// show gui
		bool bRay = false;		// show camera rays
		bool bAnimate = false;	// turn on animation features
		bool bPlayback = false; // play keyframe animation
		bool bPlayRT = false;	// render keyframe animation
		bool bKeys = false;		// show hot keys directory
		bool bRotateX = false;	// transformations
		bool bRotateY = false;
		bool bRotateZ = false;
		bool bScaleX = false;
		bool bScaleY = false;
		bool bScaleZ = false;
		bool bTranslate = false;
		bool bAltKeyDown = false;
};
