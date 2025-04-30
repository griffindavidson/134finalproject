#include "SpaceShip.h"

void SpaceShip::drawHUD(){}
void SpaceShip::updatePlayer() {
	integrate();
	// integrateRotation();
	// cam1.setPosition();
	// cam2.setPostion();
	// cam3.setPosition();
}
// might have to implement something with the heading here, not sure. Check project requirements
void SpaceShip::moveForward() { addForce(ofVec3f(1, 0, 0) * thrustPower); }
void SpaceShip::moveBackward() { addForce(ofVec3f(-1, 0, 0) * thrustPower); }
void SpaceShip::moveLeft() { addForce(ofVec3f(0, 0, 1) * thrustPower); }
void SpaceShip::moveRight() { addForce(ofVec3f(0, 0, -1) * thrustPower); }
void SpaceShip::thrustUp() { addForce(ofVec3f(0, 1, 0) * thrustPower); }
void SpaceShip::integrate() {
	// interval for this step

	float dt = 1.0 / ofGetFrameRate();
	dt = (dt > 0.0f && !std::isinf(dt)) ? dt : 1.0f / 60.0f;

	// update position based on velocity
	ofVec3f movingPosition = getPosition() += (velocity * dt);
	setPosition(movingPosition.x, movingPosition.y, movingPosition.z);
	// update acceleration with accumulated paritcles forces
	// remember :  (f = ma) OR (a = 1/m * f)
	//
	ofVec3f accel = acceleration;    // start with any acceleration already on the particle
	accel += (forces);
	velocity += accel * dt;

	// add a little damping for good measure
	//
	velocity *= damping;

	// clear forces on particle (they get re-added each step)
	//
	forces.set(0, 0, 0);
}

void SpaceShip::addForce(ofVec3f force) { forces += force; }