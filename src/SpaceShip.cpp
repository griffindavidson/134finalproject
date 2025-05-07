#include "SpaceShip.h"

void SpaceShip::drawHUD(){}
void SpaceShip::updatePlayer() {
	integrate();
	integrateRotation();
	
	cam1.setPosition(getPosition()); // first person view
	cam2.setPosition(getPosition() + glm::vec3(10,10,0));
	cam2.lookAt(getPosition()); // third person follow camera
	// cam3.setPosition(); // TBD
	light1.setPosition(getPosition());
	// light2
	// light3
	// addForce(ofVec3f(0, -1, 0) * 9.8f); // gravity force: earth gravity, a little fast for spawn height
}
// have to implement something with the heading here
void SpaceShip::moveForward() { 
	ofVec3f force = heading() * thrustPower;
	addForce(force); 
}
void SpaceShip::moveBackward() { 
	ofVec3f force = -heading() * thrustPower;
	addForce(force); }
void SpaceShip::moveLeft() { 
	ofVec3f force = heading() * thrustPower;
	glm::mat4 rotMatrix = glm::rotate(glm::mat4(1.0), glm::radians(90.0f), glm::vec3(0, 1, 0));

	// Apply rotation to the vector
	glm::vec4 rotated = rotMatrix * glm::vec4(force.x, force.y, force.z, 0.0f);
	addForce(ofVec3f(rotated.x, rotated.y, rotated.z)); 
}
void SpaceShip::moveRight() { 
	ofVec3f force = heading() * thrustPower;
	glm::mat4 rotMatrix = glm::rotate(glm::mat4(1.0), glm::radians(-90.0f), glm::vec3(0, 1, 0));

	// Apply rotation to the vector
	glm::vec4 rotated = rotMatrix * glm::vec4(force.x, force.y, force.z, 0.0f);
	addForce(ofVec3f(rotated.x, rotated.y, rotated.z));
}
void SpaceShip::thrustUp() { addForce(ofVec3f(0, 1, 0) * thrustPower); }
void SpaceShip::rotateRight() { torque = -20; }
void SpaceShip::rotateLeft() { torque = 20; }
void SpaceShip::integrate() {
	// interval for this step

	float dt = 1.0 / ofGetFrameRate();
	dt = (dt > 0.0f && !std::isinf(dt)) ? dt : 1.0f / 60.0f;

	// update position based on velocity
	// ofVec3f movingPosition = getPosition() += (velocity * dt); // ERROR CULPRIT
    
    ofVec3f movingPosition = getPosition(); // Get current position
    movingPosition += (velocity * dt);      // Add velocity * dt to it
    setPosition(movingPosition.x, movingPosition.y, movingPosition.z); // Set the new position
    
	// setPosition(movingPosition.x, movingPosition.y, movingPosition.z);
	// update acceleration with accumulated paritcles forces
	// remember :  (f = ma) OR (a = 1/m * f)
	//
	ofVec3f accel = acceleration;    // start with any acceleration already on the particle
	// cout << "Forces(x,y,z):" << forces.x << ", " << forces.y << ", " << forces.z << endl;
	accel += (forces);
	velocity += accel * dt;

	// add a little damping for good measure
	//
	velocity *= damping;

	// clear forces on particle (they get re-added each step)
	//
	forces.set(0, 0, 0);
}
void SpaceShip::integrateRotation() {
	float dt = 1.0 / ofGetFrameRate();
	dt = (dt > 0.0f && !std::isinf(dt)) ? dt : 1.0f / 60.0f;
	// Step 1: Update angular velocity with torque (a =  t / I)

	angularVelocity += torque * dt;
	// cout << "angularVelocity:" << angularVelocity << endl;
	// Step 2: Apply damping
	angularVelocity *= angularDamping;

	// Step 3: Update rotation angle
	rotation += angularVelocity * dt;
	setRotation(0, rotation, 0, 1, 0); // rotate the model according to the rotation value
	// Step 4: Clear torque for next frame
	torque = 0;

}

void SpaceShip::addForce(ofVec3f force) { forces += force; }

void SpaceShip::explode(ofVec3f startingPos, ofVec3f crashVector){
	ofVec3f movingPosition = getPosition();
	//// interpolate is used for smooth movement (less jitter) however I believe 
	//// the issue is with the speed of this function being too slow for many collided boxes
	float lerpSpeed = 0.1f; 
	movingPosition.interpolate(crashVector, lerpSpeed);
	setPosition(movingPosition.x, movingPosition.y, movingPosition.z);
} // stuff

glm::vec3 SpaceShip::heading() { // returns the heading of the player model
	
	glm::mat4 rot1 = glm::rotate(glm::mat4(1.0), glm::radians(rotation), glm::vec3(0, 1, 0));
	return glm::normalize(rot1 * glm::vec4(glm::vec3(0, 0, 1), 1));
}
