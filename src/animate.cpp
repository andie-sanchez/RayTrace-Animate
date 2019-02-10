//
//   Andie Sanchez
//   2 February 2019


//   ALL ORIGINAL CLASSES & FUNCTIONS WILL BE MARKED with " *** "

#include "ofApp.h"

//  ***
//  add new key frame for the selected object
//  
void ofApp::addKeyframe() {
	if (!selected[0]->frmExist[currFrm]) {
		selected[0]->frames[currFrm] = new Keyframe(currFrm, fnSld,
			selected[0]->position, selected[0]->rotation,
			selected[0]->scale, selected[0]->pivot);

		selected[0]->frmExist[currFrm] = true;
		recentKF[find(scene.begin(), scene.end(), selected[0]) - scene.begin()] = frmSld;
		setFrmSldColor(true);
		currFrm = frmSld;
	}
}


//  ***
//  delete key frame of selected object
//
void ofApp::delKeyframe() {
	if (selected[0]->frmExist[currFrm]) {
		Keyframe *ptr = selected[0]->frames[currFrm];

		if (ptr != NULL) {
			selected[0]->frames[currFrm] = NULL;
			delete ptr;
		}

		selected[0]->frmExist[currFrm] = false;

		int i = currFrm - 1;
		for (; i > 0; i--)
			if (selected[0]->frmExist[i])
				break;

		int j = find(scene.begin(), scene.end(), selected[0]) - scene.begin();
		recentKF[j] = i;

		setFrmSldColor(false);
		fnSld = selected[0]->frmExist[j];
	}
}


//  ***
//  update scene based on new current keyframe
//
void ofApp::updateFrame() {
	currFrm = frmSld;
	for (int j = 1; j < scene.size(); j++) {
		//get the position of the most recent frame
		int i = frmSld;
		while (!scene[j]->frmExist[i]) { i--; }
		recentKF[j] = i;
		scene[j]->position = scene[j]->frames[i]->position;
		scene[j]->rotation = scene[j]->frames[i]->rotation;
		scene[j]->scale = scene[j]->frames[i]->scale;
		//scene[j]->pivot = scene[j]->frames[i]->pivot;
	}
}


//  ***
//  changes the values of the scene object based on the current frame #
//  all easing equations are quadratic & derived from http://gizma.com/easing/
//
void ofApp::calcFrame(int i, int frame) {
	glm::vec3 p0, p1;
	Keyframe *start = scene[i]->frames[recentKF[i]], *end = scene[i]->frames[nextKF[i]];

	// ratio = time / duration
	//
	float ratio = (float)(frame - recentKF[i]) / (nextKF[i] - recentKF[i]);	

	// calculate ratio based on function
	//
	switch (start->function) {
	//case 0: break;			//linear
	case 1:						//ease-in
		ratio *= ratio;
		break;
	case 2:						//ease-out
		ratio *= -(ratio - 2);
		break;
	case 3:						//ease-in then ease-out
		ratio *= 2;
		if (ratio < 1)
			ratio *= (float) ratio / 2;
		else {
			ratio--;
			ratio = (float) -(ratio * (ratio - 2) - 1) / 2;
		}
		break;
	case 4:						//ease-out then ease-in
		ratio *= 2;
		if (ratio < 1)
			ratio *= (float) -(ratio - 2) / 2;
		else {
			ratio--;
			ratio = (float) (ratio * ratio + 1) / 2;
		}
		break;
	}

	//calculate for each channel
	//
	p0 = start->position, p1 = end->position;
	scene[i]->position = p0 + (p1 - p0) * ratio;
	p0 = start->rotation, p1 = end->rotation;
	scene[i]->rotation = p0 + (p1 - p0) * ratio;
	p0 = start->scale, p1 = end->scale;
	scene[i]->scale = p0 + (p1 - p0) * ratio;
	//p0 = start->pivot, p1 = end->pivot;
	//scene[i]->pivot = p0 + (p1 - p0) * ratio;
}


//  ***
//  advance to next frame and update scene objects  based on keyframes
//  use during playback and animation rendering
//
void ofApp::advanceFrame() {
	int done = 1;
	for (int i = 1; i < scene.size(); i++) {
		if (scene[i]->isSelectable) {
			//look if keyframe exists at current frame
			if (scene[i]->frmExist[frmCnt]) {
				recentKF[i] = frmCnt;				

				if (nextKF[i] != totalFrames) {
					int j = frmCnt + 1;
					for (; j < totalFrames; j++)
						if (scene[i]->frmExist[j])
							break;

					nextKF[i] = j;
				}
				
				//update SceneObject's values
				scene[i]->position = scene[i]->frames[frmCnt]->position;
				scene[i]->rotation = scene[i]->frames[frmCnt]->rotation;
				scene[i]->scale = scene[i]->frames[frmCnt]->scale;
				//scene[i]->pivot = scene[i]->frames[frmCnt]->pivot;
			}
			else {
				//keyframe does not exist, must calculate channel values
				if (nextKF[i] != totalFrames)
					calcFrame(i, frmCnt);
				else
					done++;
			}
		}
	}

	if(done==scene.size()) 
		frmCnt = totalFrames - 1;

	if (frmCnt == totalFrames-1) {
		frmCnt = 0;
		for (int k = 0; k < nextKF.size(); k++) 
			nextKF[k] = 0;
		
		if (bPlayRT) { 
			bPlayRT = false; 
			std::cout << "Rendering complete!" << endl; 
		}
	}
	else 
		frmCnt++;

	frmSld = frmCnt;
}