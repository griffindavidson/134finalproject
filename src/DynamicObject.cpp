//
//  DynamicObject.cpp
//  surfacetest
//
//  Created by Griffin Davidson on 5/11/25.
//

#include "DynamicObject.h"

DynamicObject::DynamicObject(std::string file) {
    object.load(file);
    position.set(0);
    velocity.set(0);
    acceleration.set(0);
    forces.set(0);
    scale.set(1);
    translation.set(0);
    angularAccel = 0;
    angularVel = 0;
    angle = 0;
    paused = false;
    positionLocked = false;
    updateBounds();
}

void DynamicObject::setGlobalForce(ofVec3f force) {
    global = force;
}

void DynamicObject::applyForce(ofVec3f force) {
    forces += force;
}

void DynamicObject::applyRotationForce(int torque) {
    angularAccel += torque;
}

void DynamicObject::update() {
    object.setPosition(position.x, position.y, position.z);
    updateBounds();
    if (paused) return;
    integrate();
    rotationIntegrate();
}


void DynamicObject::draw() {
    ofPushMatrix();
    object.setScale(scale.x, scale.y, scale.z);
    object.setRotation(0, angle, 0, 1, 0);
    object.drawFaces();
    ofPopMatrix();
}

void DynamicObject::integrate() {
    // F = M * A
    // A = F / M
    // V = A * dt
    // P = V * dt
    // can assume M = 1 unit
    
    const double dt = ofGetLastFrameTime();
    acceleration = forces + global;
    
    velocity += acceleration * dt;
    if (!positionLocked) position += velocity * dt;
    
    forces.set(0); // clear forces for next frame, ignoring global
}

void DynamicObject::rotationIntegrate() {
    
    const double dt = ofGetLastFrameTime();
    
    angularVel += angularAccel * dt;
    angularVel *= 0.99;
    angle += angularVel * dt;
    
    angularAccel = 0;
    
}

void DynamicObject::pause() {
    paused = true;
}

void DynamicObject::play() {
    paused = false;
}

void DynamicObject::lockPosition() {
    positionLocked = !positionLocked;
}

void DynamicObject::drawBoundingBox() {
    ofPushMatrix();
    ofSetColor(ofColor::white);
    Vector3 min = bounds.parameters[0];
    Vector3 max = bounds.parameters[1];
    Vector3 size = max - min;
    Vector3 center = size / 2 + min;
    ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
    float w = size.x();
    float h = size.y();
    float d = size.z();
    ofDrawBox(p, w, h, d);
    ofPopMatrix();
}

void DynamicObject::updateBounds() {
    ofVec3f sceneMin = object.getSceneMin();
    ofVec3f sceneMax = object.getSceneMax();
    
    ofVec3f scaledMin = sceneMin + position;
    ofVec3f scaledMax = sceneMax + position;

    // rover bounds are kinda weird..... this works close enough
    bounds = Box(Vector3(scaledMin.x - 10.0, scaledMin.y - 4.0, scaledMin.z - 10.0),
                 Vector3(scaledMax.x + 10.0, scaledMax.y + 5.0, scaledMax.z + 10.0));
}


Ship::Ship() : DynamicObject("geo/SpaceShipV1.obj") {
    engineSound.load("sounds/engine.mp3");
    engineSound.setVolume(0.25f);
    engineSound.setLoop(true);

    initialize(); // shared config
}

void Ship::reset() {
    velocity.set(0, 0, 0);
    angularVel = 0;

    initialize(); // shared config
}


void Ship::initialize() {
    
#include <random>

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(-200, 200);

    int randomX = dist(gen);
    int randomZ = dist(gen);
    
    position.set(randomX, 200, randomZ);
    positionLocked = false;
    pause();
    thrust = 10; // Zander: doubled fuel time after play testing
    
    // shared config for emitter + dynamics
    engine.setOneShot(false);
    engine.fired = false; // reset explosion state
    engine.setEmitterType(DirectionalEmitter);
    engine.setParticleRadius(2.5);
    engine.setVelocity(glm::vec3(0, -20, 0));
    engine.setGroupSize(1);
    
    engine.sys->forces.clear();
    engine.sys->addForce(new GravityForce(glm::vec3(0, -7, 0)));
    engine.sys->addForce(new TurbulenceForce(glm::vec3(-3), glm::vec3(3)));

    engine.visible = false;
    engine.start();
}

void Ship::update() {
    engine.setPosition(position);
    object.setPosition(position.x, position.y, position.z);
    if (paused && engine.started) engine.stop();
    if (!paused && !engine.started) engine.start();
    DynamicObject::update();
    engine.update();
    ofSoundUpdate();
    // particle emitter stuff here too
}

void Ship::draw() {
    DynamicObject::draw();
    // tried to implement shaders, did not go well, nothing worked
    //ofDisableLighting();
    engine.draw();
    //ofEnableLighting();
}

void Ship::crash() {
    lockPosition();
    
    engine.sys->addForce(new ImpulseRadialForce(1000.0));
    
    engine.setEmitterType(RadialEmitter);
    engine.setParticleRadius(2.5);
    engine.setVelocity(ofVec3f(0, 0, 0));
    engine.setGroupSize(1000);
    engine.setOneShot(true);
    engine.setPosition(position);
    
    engine.sys->reset();
    engine.start();
}

glm::vec3 Ship::heading() {
    glm::mat4 rot1 = glm::rotate(glm::mat4(1.0), glm::radians(angle), glm::vec3(0, 1, 0));
    return -glm::normalize(rot1 * glm::vec4(glm::vec3(0, 0, 1), 1));
}
