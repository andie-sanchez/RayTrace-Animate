//
//   Andie Sanchez
//   2 February 2019


//   ALL ORIGINAL CLASSES & FUNCTIONS WILL BE MARKED with " *** "

#include "ofApp.h"


//  ***
//  raytracing function: determines the color values for every pixel 
//  by iterating through each object in the scene
//
void ofApp::raytrace() {
	float width, height, w_div, h_div, w, h, dist, near_dist;
	glm::vec3 pt, norm, near_pt, near_norm, dNm, pNm;
	bool hit, shadow, inSL;
	int near_obj;
	ofColor shade;

	width = renderCam.view.width();
	height = renderCam.view.height();
	w_div = 1 / width;
	h_div = 1 / height;

	//get N samples of points in the area light
	//
	glm::vec3 pts[2][100];
	float u, v;

	//for every light in the scene
	//
	for (size_t l = 0; l < lights.size(); l++) {
		if (lights[l]->type == 2) {
			for (int i = 0; i < lights[l]->N; i++) {
				//get random u,v value b/w [0,1]
				//
				u = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				v = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				pts[l][i] = lights[l]->area.toWorld(u, v);
			}
		}
	}

	//start at bottom left
	//for every pixel
	//
	for (float i = 1; i <= width; i++) {
		for (float j = 1; j <= height; j++) {
			// get the ray at the pixel's center from the camera
			//
			w = w_div * i - w_div / 2;
			h = h_div * j - h_div / 2;
			Ray ray = renderCam.getRay(w, h);

			// default values
			//
			hit = false;
			near_dist = std::numeric_limits<float>::infinity();
			near_obj = -1;

			//for every object in the scene
			//
			for (size_t k = 0; k < scene.size(); k++) {
				//object is not a light
				if (typeid(*scene[k]) != typeid(Light)) {	
					//check if ray intersects object
					if (scene[k]->intersect(ray, pt, norm, renderCam.position)) {
						dist = glm::length(pt - renderCam.position);

						// found a closer object, update nearest object values
						if (dist < near_dist) {
							near_dist = dist;
							near_obj = k;
							near_norm = norm;
							near_pt = pt;
						}

						hit = true;
					}
				}
			} //end nearest object

			//object intersected with the view ray, add shading
			//
			if (hit) {
				//default shading with ambient lighting
				shade = scene[near_obj]->diffuseColor * ambientColor;

				//for every light in the scene
				//
				for (size_t l = 0; l < lights.size(); l++) {
					//area light, soft shadows
					//
					if (lights[l]->type == 2) {
						//store all the computed shades
						glm::vec3 shd = glm::vec3(0, 0, 0);
						ofColor temp;

						//calculate shade for every sample light point
						//
						for (int n = 0; n < lights[l]->N; n++) {
							//compute a shadow ray for each sample
							Ray shadow_ray = Ray(near_pt, glm::normalize(pts[l][n] - near_pt));

							//to determine if a shadow is cast on near_obj, check if shadow_ray intersects with any other object
							shadow = false;

							for (size_t m = 0; m < scene.size(); m++) {
								//for any other object
								if (near_obj != m) 
									//if ray intersects object, shadow exists
									if (scene[m]->intersect(shadow_ray, pt, norm, renderCam.position)) {
										shadow = true;

										if (typeid(*scene[m]) == typeid(Cube)) {
											//check if box is further away from the light than near_obj
											float distM = glm::length(pt - lights[l]->getPosition());
											float distN = glm::length(near_pt - lights[l]->getPosition());
											if (distM > distN)
												shadow = false;
										}

										break;
									}
							}

							//no shadow detected, calculate phong shading
							//
							if (!shadow) {
								temp = phong(ray.d, near_norm, l, scene[near_obj]->diffuseColor, scene[near_obj]->specularColor);
								shd += glm::vec3(temp.r, temp.g, temp.b);
							}
						}

						//calculate the average of all the computed shades
						shd = shd / lights[l]->N;

						//convert to a color and add to the ambient shade
						shade += ofColor(shd.x, shd.y, shd.z);
					}

					//point or spot light, hard shadows only
					//
					else {
						//create ray from the nearest point of intersection from the raytrace to the light's position
						Ray shadow_ray = Ray(near_pt, glm::normalize(lights[l]->getPosition() - near_pt));

						//to determine if a shadow is cast on near_obj, check if shadow_ray intersects with any other object
						shadow = false;

						for (size_t m = 0; m < scene.size(); m++) {
							//for any other object that is not a light
							if (near_obj != m && typeid(*scene[m]) != typeid(Light)) 
								//if ray intersects object, shadow exists
								if (scene[m]->intersect(shadow_ray, pt, norm, renderCam.position)) {
									shadow = true;

									if (typeid(*scene[m]) == typeid(Cube)) {
										//check if box is further away from the light than near_obj
										float distM = glm::length(pt - lights[l]->getPosition());
										float distN = glm::length(near_pt - lights[l]->getPosition());
										if (distM > distN)
											shadow = false;
									}

									break;
								}
						}

						//no shadow detected, calculate phong shading
						//
						if (!shadow){
							//spoitlight
							//
							if (lights[l]->type == 1) {
								//calculate if shadow_ray is within the spotlight breadth
								inSL = false;
								dNm = glm::normalize(lights[l]->direction);
								pNm = glm::normalize(lights[l]->getPosition() - near_pt);
								if (glm::dot(dNm, pNm) < glm::cos(lights[l]->angle + 3.15)) inSL = true;
							}

							//point light or within spotlight
							//
							if(lights[l]->type==0 || inSL)
								shade += phong(ray.d, near_norm, l, scene[near_obj]->diffuseColor, scene[near_obj]->specularColor);							
						}
					} //end if light type
				} //end lights for loop

				//shading calculated, set pixel to shade color
				image.setColor(i - 1, height - j, shade);
			} //end hit true

			//no object has an intersection at this pixel, set to background color
			//
			else 
				image.setColor(i - 1, height - j, bkgndColor);
		} //end j loop
	} //end i loop
	
	image.update();

	//save in data folder
	//	
	boost::filesystem::path fp = boost::filesystem::current_path();
	string file = fp.string();
	if (bPlayback)
		file += "//data//Animation_" + to_string(foldCnt) + "//frame";
	else
		file += "//data//frame";

	string filename = file + "_0.png";
	string p(filename);

	int i = 0;
	while (boost::filesystem::exists(p) && i < totalFrames) {
		stringstream s;
		s << file << "_" << ++i<< ".png";
		p = s.str();
	}

	image.save(p);
}


//  ***
//  Lambert Shading function
//  calculates diffuse shading
//
ofColor ofApp::lambert(const glm::vec3 &p, const glm::vec3 &norm, int i, const ofColor diffuse) {
	
	float r, I, dot_prod;

	r = glm::distance(p, lights[i]->getPosition());
	I = lights[i]->intensity / (r * r);
	dot_prod = std::max(0.0f, glm::dot(glm::normalize(norm), glm::normalize(lights[i]->getPosition())));
	
	//calculate the shading of the diffuse color
	ofColor shade = diffuse * (I * dot_prod) * lights[i]->diffuseColor;

	return shade;
}

//  ***
//  Blinn-Phong Shading function
//  calculates specular and diffuse shading (uses lambert)
//
ofColor ofApp::phong(/*const glm::vec3 &p,*/ const glm::vec3 &v, const glm::vec3 &norm, int i, const ofColor diffuse, const ofColor specular/*, float power*/) {
	
	glm::vec3 refl = glm::reflect(glm::normalize(lights[i]->getPosition()), glm::normalize(norm));
	float pw = glm::pow(std::max(0.0f, glm::dot(refl, glm::normalize(v))), lights[i]->power);
	float r = glm::distance(v, glm::vec3(lights[i]->getPosition()));
	float I = lights[i]->intensity / (r * r);
	
	//calculate the shading of the specular & diffuse colors combined
	ofColor shine = specular * I * pw * lights[i]->diffuseColor;	
	ofColor shade = lambert(v, norm, i, diffuse);

	return (shine + shade);
}