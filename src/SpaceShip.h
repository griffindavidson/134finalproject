#pragma once
#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ParticleEmitter.h"
// #include "SoundManager.cpp" if we need sound

class SpaceShip : public ofxAssimpModelLoader {
public:
	// use position from Model Loader
	float fuel; // represents the time left for fuel if < 0 then no fuel left use "ofGetElapsedTime" calls
	float thrustPower; // defines how powerful the thrust is;
	bool thrusting; // use this to decrement the fuel Example: if thrusting fuel -1;
	ofEasyCam cam1, cam2, cam3; // camera views of the lander Example: straight down, orbit, first person etc.
	ofLight light1, light2, light3; // lights on the lander that follow it
	bool isAlive; // used for state
	ParticleEmitter *thrustEmitter;
	float score; // keep track of the player's score
	// physics variables
	ofVec3f velocity;
	ofVec3f acceleration;
	ofVec3f forces;
	float	damping;
	// SoundManager& sm; if we need sound

	// constructors
	SpaceShip() {}

	~SpaceShip() {
		delete thrustEmitter;
	}

	// methods
	// draw function in modelLoader for the lander itself
	void drawHUD(); // use this to draw the fuel and altimeter to the screen
	void updatePlayer(); // this will be called in the update function, this should implement integrate
	void integrate(); // integrate the spaceship with physics

	// movement functions should add forces to the spacecraft which will be taken into account in the integrate function
	void moveForward();
	void moveLeft();
	void moveRight();
	void moveBackward();
	void thrustUp(); // gravity is automatic but upwards needs to be controlled
	void addForce(ofVec3f force);

	// do we need to implement rotation?
	float angularVelocity;  // radians/sec
	float torque;           // rotational force
	float angularDamping = 0.99f; // damping to slow rotation over time

	bool alive() { return isAlive; }
	void addScore(); // add score to the player when they successfully land in a landing zone
	void explode(); // use this to animate the lander exploding when the player fails and dies
	void reset(); // reset the player to their starting position and values to restart the game

	// For Reference this was the heading function in the 2d games
	//glm::vec3 heading() {
	//glm::mat4 rot1 = glm::rotate(glm::mat4(1.0), glm::radians(rotation), glm::vec3(0, 0, 1));
	//return glm::normalize(rot1 * glm::vec4(glm::vec3(0, -1, 0), 1));


};