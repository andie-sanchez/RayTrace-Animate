//
//   Andie Sanchez
//   2 February 2019


//   ALL ORIGINAL CLASSES & FUNCTIONS WILL BE MARKED with " *** "

#include "ofApp.h"

//  ***
//  create a new object and add it to the scene
//
void ofApp::newObj(char key) {
	// create new object at mouse position
	//
	glm::vec3 p;
	mouseToDragPlane(ofGetMouseX(), ofGetMouseY(), p);

	SceneObject *o;

	switch (key) {
	case 'b':									// block = cube
		o = new Cube(p);
		o->name = "Block" + to_string(numObj);
		break;
	case 's':									// sphere
		o = new Sphere(p);
		o->name = "Sphere" + to_string(numObj);
		break;
	case 'm':									// load default star mesh		
		model.loadModel("star.obj");			// inside ~/bin/data/
		o = new Mesh(model.getMesh(0), p);
		o->name = "Mesh" + to_string(numObj);
		break;
	case 'l':									// light
		lights.push_back(new Light(p));			// push onto lights vector
		o = lights[lights.size() - 1];
		o->name = "Light" + to_string(numObj);
		break;
	}

	// initialize keyframe 0
	// all objects have at least this keyframe
	//
	o->frames[0] = new Keyframe(0, 0, p, glm::vec3(0), glm::vec3(1), glm::vec3(0));
	o->frmExist[0] = true;

	// add this object to the parent's childList
	//
	if (objSelected()) {
		o->position = glm::inverse(selected[0]->getMatrix()) * glm::vec4(p, 1);
		selected[0]->addChild(o);
	}

	// push onto vectors
	//
	scene.push_back(o);
	recentKF.push_back(0);
	nextKF.push_back(0);
	numObj++;
}


//  ***
//  delete object and remove it from the scene
//
void ofApp::delObj() {
	SceneObject *delSel = selected[0];
	SceneObject *parent = delSel->parent;
	vector<SceneObject*>::iterator it;

	// if parent exists, update parent's childList & move delSel's children to parent
	//
	if (parent != NULL) {
		if (delSel->childList.size()) {
			parent->addChild(delSel->childList[0]);
			glm::mat4 m = delSel->getMatrix() * glm::inverse(parent->getMatrix());

			delSel->childList[0]->position = m * glm::vec4(delSel->childList[0]->position, 1);
			delSel->childList[0]->rotation = m * glm::vec4(delSel->childList[0]->rotation, 1);
			//delSel->childList[0]->pivot = m * glm::vec4(delSel->childList[0]->pivot, 1);

			if (delSel->childList.size() > 1)
				for (int i = 1; i < delSel->childList.size(); i++) {
					delSel->childList[i]->position = m * glm::vec4(delSel->childList[i]->position, 1);
					delSel->childList[i]->rotation = m * glm::vec4(delSel->childList[i]->rotation, 1);
					//delSel->childList[i]->pivot = m * glm::vec4(delSel->childList[i]->pivot, 1);
					parent->addChild(delSel->childList[i]);
				}
		}

		it = find(parent->childList.begin(), parent->childList.end(), delSel);
		if (it != parent->childList.end())
			parent->childList.erase(it);
	}

	// if parent does not exist, change parent node of delSel's children
	//
	else {
		if (delSel->childList.size()) {
			parent = delSel->childList[0]; //new parent
			parent->parent = NULL;
			parent->position = delSel->getMatrix() * glm::vec4(parent->position, 1);
			parent->rotation = delSel->getMatrix() * glm::vec4(parent->rotation, 1);
			//parent->pivot = delSel->getMatrix() * glm::vec4(parent->pivot, 1);

			// has more children, make them the new root's children
			//
			if (delSel->childList.size() > 1) {
				glm::mat4 m = delSel->getMatrix() * glm::inverse(parent->getMatrix());
				for (int i = 1; i < delSel->childList.size(); i++) {
					delSel->childList[i]->position = m * glm::vec4(delSel->childList[i]->position, 1);
					delSel->childList[i]->rotation = m * glm::vec4(delSel->childList[i]->rotation, 1);
					//delSel->childList[i]->pivot = m * glm::vec4(delSel->childList[i]->pivot, 1);
					parent->addChild(delSel->childList[i]);
				}
			}
		}
	}

	// remove delSel from the scene
	//
	it = find(scene.begin(), scene.end(), delSel);
	if (it != scene.end()) {
		recentKF.erase(recentKF.begin() + (it - scene.begin()));
		nextKF.erase(nextKF.begin() + (it - scene.begin()));
		scene.erase(it);
	}

	// object is a light, remove from light vector
	// 
	if (typeid(*delSel) == typeid(Light)) {
		vector<Light*>::iterator l;
		l = find(lights.begin(), lights.end(), delSel);
		if (l != lights.end())
			lights.erase(l);
	}

	// clear selection & delete selected
	//
	selected.clear();
	delete delSel;
}


//  ***
//  if the object selection has changed, update slider values
//
void ofApp::updateSliders() {
	prevSelected = selected[0];	// to track if selection changes

	// color sliders
	//
	rSld = selected[0]->diffuseColor.r;
	gSld = selected[0]->diffuseColor.g;
	bSld = selected[0]->diffuseColor.b;

	// animation sliders
	//
	if (bAnimate && !bPlayback) {
		int i = recentKF[find(scene.begin(), scene.end(), selected[0]) - scene.begin()];
		fnSld = selected[0]->frames[i]->function;
		setFrmSldColor(selected[0]->frmExist[currFrm]);
	}

	// light sliders
	//
	if (typeid(*selected[0]) == typeid(Light)) {
		Light *l = (Light*)selected[0];
		typeSld = l->type;
		pwrSld = l->power;
		intstSld = l->intensity;
		splDegSld = l->angle;
		NSld = l->N;
		widthSld = l->area.width();
		heightSld = l->area.height();
	}
	else
		typeSld = pwrSld = intstSld = splDegSld = NSld = widthSld = heightSld = -1;
}


//  ***
//  update selected[0]'s values based on changed sliders
//
void ofApp::updateSelected(bool newFrm) {
	// color sliders
	//
	if (rSld != selected[0]->diffuseColor.r)		selected[0]->diffuseColor.r = rSld;
	else if (gSld != selected[0]->diffuseColor.g)	selected[0]->diffuseColor.g = gSld;
	else if (bSld != selected[0]->diffuseColor.r)	selected[0]->diffuseColor.b = bSld;

	// animation sliders
	//
	if (bAnimate && !bPlayback) {
		int i = recentKF[find(scene.begin(), scene.end(), selected[0]) - scene.begin()];

		if (newFrm) {
			fnSld = selected[0]->frames[i]->function;
			setFrmSldColor(selected[0]->frmExist[currFrm]);
		}
		else if (selected[0]->position != selected[0]->frames[i]->position)
			selected[0]->frames[i]->position = selected[0]->position;
		else if (selected[0]->rotation != selected[0]->frames[i]->rotation)
			selected[0]->frames[i]->rotation = selected[0]->rotation;
		else if (selected[0]->scale != selected[0]->frames[i]->scale)
			selected[0]->frames[i]->scale = selected[0]->scale;
		//else if (selected[0]->pivot != selected[0]->frames[i]->pivot)
			//selected[0]->frames[i]->pivot = selected[0]->pivot;
		else if (fnSld != selected[0]->frames[i]->function)
			selected[0]->frames[i]->function = fnSld;
	}

	// light sliders
	//
	if (typeid(*selected[0]) == typeid(Light)) {
		Light *l = (Light*)selected[0];
		if (typeSld != l->type)				l->type = typeSld;
		else if (pwrSld != l->power)		l->power = pwrSld;
		else if (intstSld != l->intensity)	l->intensity = intstSld;
		else if (splDegSld != l->angle)		l->angle = splDegSld;
		else if (NSld != l->N)				l->N = NSld;
		else if (widthSld != l->area.width())
			l->area.setSize(glm::vec2(-widthSld / 2, l->area.min.y), glm::vec2(widthSld / 2, l->area.max.y));
		else if (heightSld != l->area.height())
			l->area.setSize(glm::vec2(l->area.min.x, -heightSld / 2), glm::vec2(l->area.max.x, heightSld / 2));
	}
}


//  ***
//  if keyframe exists, highlight the frame slider green
//
void ofApp::setFrmSldColor(bool exists) {
	if (exists) {
		frmSld.setFillColor(ofColor::lightGreen);
		frmSld.setBackgroundColor(ofColor::darkGreen);
	}
	else {
		frmSld.setFillColor(ofColor::lightGray);
		frmSld.setBackgroundColor(ofColor::darkGray);
	}
}


// Draw an XYZ axis in RGB at transform
//
void ofApp::drawAxis(glm::mat4 m, float len) {
	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(len, 0, 0, 1)));

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(0, len, 0, 1)));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(0, 0, len, 1)));
}


// print C++ code for obj tranformation channels. (for debugging);
//
void ofApp::printChannels(SceneObject *obj) {
	cout << obj->name << ":" << endl;
	cout << "position = glm::vec3(" << obj->position.x << "," << obj->position.y << "," << obj->position.z << ");" << endl;
	cout << "rotation = glm::vec3(" << obj->rotation.x << "," << obj->rotation.y << "," << obj->rotation.z << ");" << endl;
	cout << "scale = glm::vec3(" << obj->scale.x << "," << obj->scale.y << "," << obj->scale.z << ");\n" << endl;
}