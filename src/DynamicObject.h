//
//  DynamicObject.h
//  surfacetest
//
//  Created by Griffin Davidson on 5/11/25.
//

#include "ofxAssimpModelLoader.h"
#include "Particles/ParticleEmitter.h"
#include "Octree/Dependencies/box.h"

class DynamicObject {
public:
    ofxAssimpModelLoader object;
    ofVec3f forces, global, position, velocity, acceleration, scale, translation;
    float angularAccel, angularVel, angle;
    Box bounds;
    
    DynamicObject(std::string file);
    
    void setGlobalForce(ofVec3f force);
    void applyForce(ofVec3f force);
    void applyRotationForce(int force);
    void pause(); // pauses all physics
    void play(); // resumes all physics
    void lockPosition(); // prevents rover from moving, used for crash()
    
    void drawBoundingBox();
    virtual void update();
    virtual void draw();
    
protected:
    bool paused;
    bool positionLocked;
    
    void updateBounds();
    void integrate(); // physics integration
    void rotationIntegrate(); // y-axis rotation physics
    
};

class Ship: public DynamicObject {
public:
    //ParticleEmitter engine; implement later
    Ship();
    float thrust;
    ParticleEmitter engine;
    ofSoundPlayer engineSound;
    ofShader shader;
    
    bool isPaused() { return paused; }
    void update() override;
    void draw() override;
    void reset();
    void crash();
private:
    void initialize();
};


