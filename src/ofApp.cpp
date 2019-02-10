#include "ofApp.h"

//  ***
//  initialize the 3D space for use
//
void ofApp::setup(){
	// set default scene
	//
	srand(time(NULL));
	ofSetBackgroundColor(bkgndColor);

	// plane cannot be created/deleted during runtime
	//
	scene.push_back(new Plane(glm::vec3(0, -2, 0)));
	scene[0]->name = "Plane0";
	scene[0]->frmExist[0] = true;
	scene[0]->frames[0] = new Keyframe(0, 0, glm::vec3(0, -2, 0), glm::vec3(0), glm::vec3(1), glm::vec3(0));
	recentKF.push_back(0);
	nextKF.push_back(0);

	// initialize pixel size of renderCam and image
	//
	imgW = 1200;
	imgH = 800;
	image.allocate(imgW, imgH, OF_IMAGE_COLOR);
	renderCam.setSize(glm::vec2(-imgW / 2, -imgH / 2), glm::vec2(imgW / 2, imgH / 2));
	
	//  set cameras
	//
	mainCam.setDistance(15);
	mainCam.setNearClip(.1);

	sideCam.setPosition(20, 0, 0);
	sideCam.lookAt(glm::vec3(0, 0, 0));

	topCam.setNearClip(.1);
	topCam.setPosition(0, 16, 0);
	topCam.lookAt(glm::vec3(0, 0, 0));

	rndrCam.setPosition(renderCam.position);
	rndrCam.setNearClip(.1);
	rndrCam.setFov(61.927); // FOV for 1200x800 viewplane size with ratio of 0.005 & distance of 5

	theCam = &mainCam;

	// set gui sliders 
	//

	// object sliders
	//
	gui.setup();
	gui.setName("Object Sliders");
	gui.setPosition(10, 30);

	gui.add(rSld.setup("Red", 0, 0, 255));
	gui.add(gSld.setup("Green", 0, 0, 255));
	gui.add(bSld.setup("Blue", 0, 0, 255));

	rSld.setFillColor(ofColor(255, 0, 0));
	gSld.setFillColor(ofColor(0, 255, 0));
	bSld.setFillColor(ofColor(0, 0, 255));
	
	// lighting sliders
	//
	gui.add(typeSld.setup("Lighting Type", -1, 0, 2));
	gui.add(pwrSld.setup("Power", -1, 1, 50));
	gui.add(intstSld.setup("Intensity", -1, 0, 50));
	
	gui.add(splDegSld.setup("Spotlight Breadth", -1, 0, 1));
	gui.add(NSld.setup("Number Samples", -1, 1, 100));
	gui.add(widthSld.setup("Area Width", -1, 1, 8));
	gui.add(heightSld.setup("Area Height", -1, 1, 8));

	// animation sliders
	//
	animGui.setup();
	animGui.setName("Animation Sliders");
	animGui.setPosition(230, 30);
	animGui.setSize(ofGetWindowWidth()-250, 100);

	animGui.add(frmSld.setup("Frame", 0, 0, totalFrames-1, ofGetWindowWidth()-250));
	animGui.add(fnSld.setup("In-between frame Function", 0, 0, 4, 250));
	setFrmSldColor(true);
}

//  ***
//  update scene channels and sliders
//
void ofApp::update(){
	// playback on, aniamte scene
	//
	if (bAnimate && bPlayback) { 
		if (bPlayRT) raytrace();
		advanceFrame();
	}
	// playback off, adjust scene & guis
	//
	else {
		bool newFrm = false;

		// update all SceneObject's channels based on the new current frame
		//
		if (bAnimate && !bPlayback && frmSld != currFrm) {
			updateFrame(); 
			newFrm = true;
		}
		
		// update selected object
		//
		if (objSelected())
			if (prevSelected != selected[0]) updateSliders();
			else updateSelected(newFrm);
		
		// nothing selected, update background & set to defaults
		//
		else {
			if(prevSelected==NULL){
				if (rSld != bkgndColor.r)		bkgndColor.r = rSld;
				else if (gSld != bkgndColor.g)	bkgndColor.g = gSld;
				else if (bSld != bkgndColor.r)	bkgndColor.b = bSld;
				ofSetBackgroundColor(bkgndColor);
			}else{
				prevSelected = NULL;
				rSld = bkgndColor.r;
				gSld = bkgndColor.g;
				bSld = bkgndColor.b;
				typeSld = pwrSld = intstSld = splDegSld = NSld = widthSld = heightSld = -1;				
			}
			if (bAnimate && !bPlayback) { 
				setFrmSldColor(false); 
				fnSld = -1; 
			}
		}
	}
}


//  ***
//  draw all elements of the 3D space
//
void ofApp::draw(){
	theCam->begin();
	ofNoFill();	

	//  draw the objects in scene
	//
	material.begin();
	ofFill();

	for (int i = 0; i < scene.size(); i++) {
		ofSetColor(scene[i]->diffuseColor);
		scene[i]->draw();

		if (objSelected() && scene[i] == selected[0]) {
			ofSetColor(ofColor::white);
			scene[i]->drawEdges();
		}
	}

	// draw the rendered image on the viewplane when bHide is false
	//
	if (!bImage) {
		ofSetColor(ofColor::white, 255);
		float iw = imgW * renderCam.view.rt, ih = imgH * renderCam.view.rt;
		image.draw(renderCam.view.position.x - iw / 2, 
			renderCam.view.position.y - ih / 2, 
			renderCam.view.position.z, iw, ih);
	}
	
	drawAxis();

	ofNoFill();
	ofSetColor(ofColor::lightSkyBlue);
	renderCam.drawFrustum();
	ofSetColor(ofColor::blue);
	renderCam.draw();	
	
	material.end();
	theCam->end();

	ofSetColor(ofColor::white);
	ofDrawBitmapString("For Controls Directory press H", 10, 20);

	if(!bHide) gui.draw();
	if(bAnimate) animGui.draw();

	if (bKeys) {
		ofFill();
		ofSetColor(ofColor::black);
		ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
		ofSetColor(ofColor::white);
		ofDrawBitmapString(
			"HOT KEYS:\n\n"
			"MAIN FUNCTIONS:\n"
			"ALT = enable object selection\n"
			"A   = enable animation interface\n"
			"B   = playback\n"
			"R   = ray trace\n"
			"to render animation, press R when playback is on\n"
			"O   = print channels of selected object\n"
			"if no object is selected, all objects\' channels printed\n\n"
			"SCENE OBJECTS:\n"
			"SHIFT + B = create block\n"
			"SHIFT + S = create sphere\n"
			"SHIFT + M = create star triangle mesh\n"
			"SHIFT + L = create light\n"
			"D         = delete selected object\n"
			"K         = create keyframe\n"
			"SHIFT + K = delete keyframe\n\n"
			"TRANSFORM:\n"
			"drag selected object and hold hot key\n"
			"X          = rotate x\n"
			"Y          = rotate y\n"
			"Z          = rotate z\n"
			"SHIFT + X  = scale x\n"
			"SHIFT + Y  = scale y\n"
			"SHIFT + Z  = scale z\n"
			"T          = translate\n\n",
			10, 30);

		ofDrawBitmapString(
			"LIGHTING TYPES:\n"
			"0 = point light\n"
			"1 = spot light\n"
			"2 = area light\n\n"
			"ANIMATION FUNCTION TYPES:\n"
			"0 = linear\n"
			"1 = ease-in\n"
			"2 = ease-out\n"
			"3 = ease-out then ease-in\n"
			"4 = ease-in then ease-out\n\n"
			"VIEW:\n"
			"CTRL = enable camera mouse input\n"
			"f = fullscreen\n"
			"h = show help directory\n"
			"g = show object sliders\n"
			"i = show rendered image\n\n"
			"CAMERA:\n"
			"F1 = main\n"
			"F2 = side\n"
			"F3 = top\n"
			"F4 = render",
			500, 55);
	}
}


void ofApp::keyReleased(int key) {
	switch (key) {
	case OF_KEY_ALT:
		bAltKeyDown = false;
		mainCam.disableMouseInput();
		break;
	case 'x': bRotateX = false; break;
	case 'y': bRotateY = false;	break;
	case 'z': bRotateZ = false;	break;
	case 'X': bScaleX = false;	break; // ***
	case 'Y': bScaleY = false;	break; // ***
	case 'Z': bScaleZ = false;	break; // ***
	case 't': bTranslate = false; break;
	default:  break;
	}
}

//  ***
//  hot keys for user interface
//
void ofApp::keyPressed(int key) {
	switch (key) {
	// start/stop playback
	//
	case 'p':
		if (bAnimate) {
			bPlayback = !bPlayback;
			if (bPlayback) {
				ofSetFrameRate(24);
				frmCnt = 0;
				for (int i = 0; i < nextKF.size(); i++) nextKF[i] = 0;
				std::cout << "Animation playback ON" << endl;
			}
			else {
				ofSetFrameRate(60);
				std::cout << "Animation playback OFF" << endl;
			}
		}
		break;

	// begin/end raytracing
	//
	case 'r':
		std::cout << "Rendering start!" << endl;
		if (!bPlayback) {
			raytrace();
			std::cout << "Rendering complete!" << endl;
		}
		else {
			if (!bPlayRT) {
				frmCnt = frmSld = currFrm = 0;
				updateFrame();
				
				boost::filesystem::path fp = boost::filesystem::current_path();
				string folder = fp.string();
				folder += "//data//Animation";
				string tempFolder = folder + "_0";
				string p(tempFolder);

				int i = 0;
				while (boost::filesystem::exists(p) && i < 50) {
					stringstream s;
					s << folder << "_" << ++i;
					p = s.str();
				}
				foldCnt = i;
			}
			bPlayRT = !bPlayRT;
		}	
		break;

	// print channels for selected object
	// if none selected print for all objects
	//
	case 'o':
		if (objSelected()) printChannels(selected[0]);
		else for (int i = 0; i < scene.size(); i++)	printChannels(scene[i]);
		break;

	// lock selected object, so that it cannot be transformed
	//
	case 'l': 
		if (objSelected() && selected[0] != scene[0]) 
			selected[0]->isLocked = !selected[0]->isLocked; 
		break;

	// add/delete keyframes
	//
	case 'k': 
		if (bAnimate && !bPlayback && objSelected()) 
			addKeyframe(); 
		break;
	case 'K': 
		if (bAnimate && !bPlayback && objSelected() && currFrm != 0) 
			delKeyframe();
		break;

	// add/delete objects
	//
	case 'B': newObj('b'); break;						//create block/cube
	case 'M': newObj('m'); break;						//create star mesh		
	case 'S': newObj('s'); break;						//create sphere		
	case 'L': newObj('l'); break;						//create light
	case 'd': 											//delete object
		if (objSelected() && !selected[0]->isLocked)	
			delObj(); 
		break;

	// change states
	//
	case 'a': bAnimate = !bAnimate; break;
	//case 't': bRay = !bRay;			break;
	case 'h': bKeys = !bKeys;		break; 
	case 'i': bImage = !bImage;		break;
	case 'g': bHide = !bHide;		break;
	case 'x': bRotateX = true;		break;
	case 'y': bRotateY = true;		break;
	case 'z': bRotateZ = true;		break;
	case 'X': bScaleX = true;		break;
	case 'Y': bScaleY = true;		break;
	case 'Z': bScaleZ = true;		break;
	case 't': bTranslate = true;	break;
	case 'f': ofToggleFullscreen(); break;

	// camera view and mouse input
	//
	case OF_KEY_F1: theCam = &mainCam; break;
	case OF_KEY_F2: theCam = &sideCam; break;
	case OF_KEY_F3: theCam = &topCam;	break;
	case OF_KEY_F4: theCam = &rndrCam; break;
	case OF_KEY_CONTROL:
		if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
		else mainCam.enableMouseInput();
		break;
	case OF_KEY_ALT: 
		bAltKeyDown = true;
		break;
	default:
		break;
	}
}

// *** scale transformations added to skeleton code
//
void ofApp::mouseDragged(int x, int y, int button) {
	if (objSelected() && bDrag && !selected[0]->isLocked) {
		glm::vec3 point;
		mouseToDragPlane(x, y, point);

		if (bRotateX) 
			selected[0]->rotation += glm::vec3((point.x - lastPoint.x) * 20.0, 0, 0);		
		else if (bRotateY) 
			selected[0]->rotation += glm::vec3(0, (point.x - lastPoint.x) * 20.0, 0);		
		else if (bRotateZ) 
			selected[0]->rotation += glm::vec3(0, 0, (point.x - lastPoint.x) * 20.0);		
		else if (bScaleX && typeid(*selected[0]) != typeid(Light))
			selected[0]->scale += glm::vec3((point.x - lastPoint.x) * 0.5, 0, 0);		
		else if (bScaleY && typeid(*selected[0]) != typeid(Light))
			selected[0]->scale += glm::vec3(0, (point.x - lastPoint.x) * 0.5, 0);		
		else if (bScaleZ && typeid(*selected[0]) != typeid(Light))
			selected[0]->scale += glm::vec3(0, 0, (point.x - lastPoint.x) * 0.5);		
		else if(bTranslate)
			selected[0]->position += (point - lastPoint);

		lastPoint = point;			
	}
}

//  This projects the mouse point in screen space (x, y) to a 3D point on a plane
//  normal to the view axis of the camera passing through the point of the selected object.
//  If no object selected, the plane passing through the world origin is used.
//
bool ofApp::mouseToDragPlane(int x, int y, glm::vec3 &point) {
	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);	
	glm::vec3 pos;
	float dist;

	if (objSelected())	pos = selected[0]->position;
	else pos = glm::vec3(0, 0, 0);

	if (glm::intersectRayPlane(p, dn, pos, glm::normalize(theCam->getZAxis()), dist)) {
		point = p + dn * dist;
		return true;
	}
	return false;
}

// Provides functionality of single selection and if something is already selected,
// sets up state for translation/rotation of object using mouse.
//
void ofApp::mousePressed(int x, int y, int button) {
	// if we are moving the camera around, don't allow selection
	if (mainCam.getMouseInputEnabled()) return;

	// clear selection list
	selected.clear();

	// test if something selected
	vector<SceneObject *> hits;

	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	// check for selection of scene objects
	for (int i = 0; i < scene.size(); i++) {
		glm::vec3 point, norm;

		//  We hit an object
		if (scene[i]->isSelectable && scene[i]->intersect(Ray(p, dn), point, norm, renderCam.position))
			hits.push_back(scene[i]);
	}

	// if we selected more than one, pick nearest
	SceneObject *selectedObj = NULL;
	if (hits.size() > 0) {
		selectedObj = hits[0];
		float nearestDist = numeric_limits<float>::infinity();
		for (int n = 0; n < hits.size(); n++) {
			float dist = glm::length(hits[n]->position - theCam->getPosition());
			if (dist < nearestDist) {
				nearestDist = dist;
				selectedObj = hits[n];
			}
		}
	}
	if (selectedObj) {
		selected.push_back(selectedObj);
		bDrag = true;
		mouseToDragPlane(x, y, lastPoint);
	}
	else
		selected.clear();
}

void ofApp::mouseReleased(int x, int y, int button) { bDrag = false; }

//  ***
//  to add mesh from files, drag and drop into the window
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	// get file path from dragInfo.files[0] & create path object
	//
	boost::filesystem::path path(dragInfo.files[0]);

	// check path.extension() if extension accepted
	//
	string ext = path.extension().string();
	glm::vec3 p;
	SceneObject *o;
	mouseToDragPlane(ofGetMouseX(), ofGetMouseY(), p);

	// load object
	//
	if (ext == ".obj" || ext == ".stl") {
		model.loadModel(dragInfo.files[0]); 

		for (int i = 0; i < model.getNumMeshes(); i++) {
			o = new Mesh(model.getMesh(i), p);
			o->name = "Mesh" + to_string(numObj);
			o->frames[0] = new Keyframe(0, 0, p, glm::vec3(0), glm::vec3(1), glm::vec3(0));
			o->frmExist[0] = true;

			if (objSelected()) {
				o->position = glm::inverse(selected[0]->getMatrix()) * glm::vec4(p, 1);
				selected[0]->addChild(o);
			}

			scene.push_back(o);
			recentKF.push_back(0);
			nextKF.push_back(0);
			numObj++;
		}

		cout << path.filename() << " has been loaded!" << endl;
	}
	else // print error message
		cout << "ERROR: " << path.extension() << " not accepted." << endl;
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {}
void ofApp::mouseEntered(int x, int y){}
void ofApp::mouseExited(int x, int y){}
void ofApp::windowResized(int w, int h){}
void ofApp::gotMessage(ofMessage msg){}