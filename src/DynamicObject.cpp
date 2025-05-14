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
    if (paused) return;
    DynamicObject::integrate();
    DynamicObject::rotationIntegrate();
    object.setPosition(position.x, position.y, position.z);
    updateBounds();
}

void DynamicObject::draw() {
    object.setRotation(0, angle, 0, 1, 0);
    object.drawFaces();
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
    position += velocity * dt;
    
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

void DynamicObject::drawBoundingBox() {
    Vector3 min = bounds.parameters[0];
    Vector3 max = bounds.parameters[1];
    Vector3 size = max - min;
    Vector3 center = size / 2 + min;
    ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
    float w = size.x();
    float h = size.y();
    float d = size.z();
    ofDrawBox(p, w, h, d);
}

void DynamicObject::updateBounds() {
    ofVec3f sceneMin = object.getSceneMin();
    ofVec3f sceneMax = object.getSceneMax();
    ofVec3f scaledMin = sceneMin * scale + position;
    ofVec3f scaledMax = sceneMax * scale + position;

    bounds = Box(Vector3(scaledMin.x, scaledMin.y, scaledMin.z),
                 Vector3(scaledMax.x, scaledMax.y, scaledMax.z));
}


Ship::Ship() : DynamicObject("geo/lander.obj") {
   // add particle emitter stuff here for thrust
    thrust = 5;
    
    GravityForce *g = new GravityForce(glm::vec3(0, -7, 0));
    TurbulenceForce *t = new TurbulenceForce(glm::vec3(-3), glm::vec3(3));
    
    engine.sys->addForce(g);
    engine.sys->addForce(t);
    
    //shader.load("shaders/particle");
    engineSound.load("sounds/engine.mp3");
    engineSound.setVolume(0.25f);
    engineSound.setLoop(true);
    
    engine.start();
    engine.visible = true;
    engine.setVelocity(glm::vec3(0, -20, 0));
    engine.setParticleRadius(5);
}

void Ship::update() {
    if (paused && engine.started) engine.stop();
    if (!paused && !engine.started) engine.start();
    DynamicObject::update();
    engine.setPosition(position);
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

