#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    // Basic setup
    ofSetEscapeQuitsApp(false);
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofEnableSmoothing();
    ofEnableDepthTest();
    ofSetBackgroundColor(30, 30, 30); // Dark gray background
    ofSetWindowShape(2100, 1300); // I'm Sick Of Small Windows! LOL

    // DEBUG Camera setup
    cam.setDistance(50); // Start much further back
    cam.setNearClip(0.1);
    cam.setFarClip(5000);
    cam.setFov(60);
    cam.enableMouseInput();
    cam.setPosition(10, 10, 10);

    // theCam = &rover3rdPersonCam;
    theCam = &cam;

    // Initialize Global lighting (sunlight)
    if (daytime) { initLightingAndMaterials(); }

    terrain.load("geo/gameTerrainV3.obj");
    terrain.setScaleNormalization(false);
    terrain.setPosition(0, 0, 0);
    octreeTerrain.bUseFaces = true;
    octreeTerrain.create(terrain.getMesh(0), 10);
    
    // lighting
    landing1light.setup();
    landing1light.enable();
    landing1light.setSpotlight();
    landing1light.setScale(.05);
    landing1light.setSpotlightCutOff(30);
    landing1light.setPosition(150, 8 + 50, 95);
    landing1light.rotate(-90,ofVec3f(1,0,0));

    landing2light.setup();
    landing2light.enable();
    landing2light.setSpotlight();
    landing2light.setScale(.05);
    landing2light.setSpotlightCutOff(30);
    landing2light.setPosition(-200, 160 + 50, 75);
    landing2light.rotate(-90, ofVec3f(1, 0, 0));

    landing3light.setup();
    landing3light.enable();
    landing3light.setSpotlight();
    landing3light.setScale(.05);
    landing3light.setSpotlightCutOff(30);
    landing3light.setPosition(0, 45+50, -250);
    landing3light.rotate(-90, ofVec3f(1, 0, 0)); // points downward at the pads

    roverLight.setup();
    roverLight.enable();
    roverLight.setSpotlight();
    roverLight.setScale(0.05);
    roverLight.setSpotlightCutOff(30);
    roverLight.setPosition(rover.position.x, rover.position.y + 50, rover.position.z);
    roverLight.rotate(-90, ofVec3f(1, 0, 0));

    roverHeadLamp.setup();
    roverHeadLamp.enable();
    roverHeadLamp.setSpotlight();
    roverHeadLamp.setScale(0.05);
    roverHeadLamp.setSpotlightCutOff(20);
    roverHeadLamp.setPosition(rover.position);
    // roverHeadLamp.rotate(-90, ofVec3f(1, 0, 0));

    loadModelAtPosition(landingPad1, "geo/gameLandingPadV1.obj", ofVec3f(150, 8, 95), landMesh1);
    landing1Cam.setPosition(glm::vec3(150+15, 8+5, 95+15)); // set camera position
    landing1Cam.disableMouseInput();
    octreePad1.bUseFaces = true;
    octreePad1.create(landMesh1, 4);
    
    loadModelAtPosition(landingPad2, "geo/gameLandingPadV1.obj", ofVec3f(-200, 160, 75), landMesh2);
    landing2Cam.setPosition(glm::vec3(-200+15, 160+5, 75+15)); //160 is good for the High vertex terrain
    landing2Cam.disableMouseInput();
    octreePad2.bUseFaces = true;
    octreePad2.create(landMesh2, 4);
    
    loadModelAtPosition(landingPad3, "geo/gameLandingPadV1.obj", ofVec3f(0, 45, -250), landMesh3);
    landing3Cam.setPosition(glm::vec3(0+15, 45+5, -250+15));
    landing3Cam.disableMouseInput();
    octreePad3.bUseFaces = true;
    octreePad3.create(landMesh3, 4);
    
    rover.scale.set(0.05);
    rover.setGlobalForce(ofVec3f(0, -1.625, 0));

    rover3rdPersonCam.setPosition(rover.position.x - 75, rover.position.y + 45, rover.position.y - 75);
    rover3rdPersonCam.lookAt(rover.position);
    rover3rdPersonCam.setNearClip(0.1);
    rover3rdPersonCam.setFarClip(1000);
    rover3rdPersonCam.setFov(65.5);
    rover3rdPersonCam.disableMouseInput();

    float roverMiddleHeight = rover.object.getSceneCenter().y;
    rover1stPersonCam.setPosition(rover.position.x, rover.position.y, rover.position.z-2); // these lines don't really do anything
    rover1stPersonCam.lookAt(-glm::vec3(rover.position.x, rover.position.y, rover.position.z - 30)); // random direction for now, make it look at normal
    rover1stPersonCam.setNearClip(7.25); // see updateCameras() for real code
    rover1stPersonCam.setFarClip(1000);
    rover1stPersonCam.setFov(65.5);
    rover1stPersonCam.disableMouseInput();

    roverTopDownCam.setPosition(rover.position.x, rover.position.y + 50, rover.position.z);
    roverTopDownCam.lookAt(rover.position);
    roverTopDownCam.setNearClip(0);
    roverTopDownCam.setFarClip(1000);
    roverTopDownCam.setFov(65.5);
    roverTopDownCam.disableMouseInput();

    gui.setup();
    gui.add(octreeLevels.set("Octree Levels", 0, 0, 20));
    gui.add(fps.setup("FPS", ""));
    gui.add(altitudeLabel.setup("Altitude", ""));
    gui.add(thrust.setup("Thrust", ""));
    gui.add(moveLabel.setup("WASD", "Move"));
    gui.add(rotateLabel.setup("Q/E", "Rotate"));
    gui.add(toggleCam.setup("C", "Toggle Cameras"));
    gui.add(pauseLabel.setup("ESC", "Pause"));
    gui.add(velocityLabel.setup("Velocity", ""));

    // cout << "Root box: (" << ofToString(octree.root.box.min().x()) << "," << ofToString(octree.root.box.min().y()) << ", " << ofToString(octree.root.box.min().z()) << ")" << " to " << "(" << ofToString(octree.root.box.max().x()) << "," << ofToString(octree.root.box.max().y()) << ", " << ofToString(octree.root.box.max().z()) << ")" << endl;
    // cout << "Number of children: " << ofToString(octree.root.children.size()) << endl;
}

//--------------------------------------------------------------
void ofApp::update(){
    updateCameras();
    rover.update();
    rover.engine.setRate(0);

    if (!rover.isPaused())
    {
        if (keymap[' '] && rover.thrust > 0.0f)
        {
            // Apply physics and play engine sound
            rover.applyForce(ofVec3f(0, 5, 0));
            rover.thrust -= 0.01;
            rover.engine.setRate(20);

            if (!rover.engineSound.isPlaying()) {
                rover.engineSound.play();
            }
        } else {
            // Stop sound if not thrusting
            if (rover.engineSound.isPlaying()) {
                rover.engineSound.stop();
            }
        }
    } else {
        // Ensure sound is stopped when paused
        if (rover.engineSound.isPlaying()) {
            rover.engineSound.stop();
        }
    }

    // "beautifies" thrust so its not some tiny number below 0
    if (rover.thrust <= 0) rover.thrust = 0.0f;
    thrust = ofToString(rover.thrust / 5.0f * 100.0f) + "%";
    velocityLabel = ofToString(rover.velocity.y) + "m/s"; // to be removed

    // glm::vec3 forwardDir, rightDir;

    // if (theCam == &roverTopDownCam) {
    //     // Top-down: "forward" is the direction the camera is pointing, flattened to XZ
    //     forwardDir = glm::normalize(theCam->getLookAtDir());
    //     forwardDir.y = 0;
    //     forwardDir = glm::normalize(forwardDir);
    //     rightDir = glm::normalize(glm::cross(forwardDir, glm::vec3(0,1,0)));
    // } else {
    //     // 1st-person or 3rd-person: Use camera direction
    //     forwardDir = glm::normalize(theCam->getLookAtDir());
    //     forwardDir.y = 0;
    //     forwardDir = glm::normalize(forwardDir);
    //     rightDir = glm::normalize(glm::cross(forwardDir, glm::vec3(0,1,0)));
    // }

    // Movement input
    float moveSpeed = 2;

    if (keymap['w'])
    {
        rover.applyForce(rover.heading() * moveSpeed);
    }
    if (keymap['s'])
    {
        rover.applyForce(-rover.heading() * moveSpeed);
    }
    if (keymap['a'])
    {
        ofVec3f force = rover.heading() * moveSpeed;
        glm::mat4 rotMatrix = glm::rotate(glm::mat4(1.0), glm::radians(90.0f), glm::vec3(0, 1, 0));

        // Apply rotation to the vector
        glm::vec4 rotated = rotMatrix * glm::vec4(force.x, force.y, force.z, 0.0f);
        rover.applyForce(ofVec3f(rotated.x, rotated.y, rotated.z));
    }
    if (keymap['d'])
    {
        ofVec3f force = rover.heading() * moveSpeed;
        glm::mat4 rotMatrix = glm::rotate(glm::mat4(1.0), glm::radians(-90.0f), glm::vec3(0, 1, 0));

        // Apply rotation to the vector
        glm::vec4 rotated = rotMatrix * glm::vec4(force.x, force.y, force.z, 0.0f);
        rover.applyForce(ofVec3f(rotated.x, rotated.y, rotated.z));
    }

    if (keymap['q'])
    { // ROTATE LEFT
        rover.applyRotationForce(15);
    }
    
    if (keymap['e']) { // ROTATE RIGHT
        rover.applyRotationForce(-15);
    }

    // Zander: need to change altitude function so that it uses octree.intersect(ray)

    altitudeLabel = ofToString(getAltitude()) + "m";

    // COLLISION V2 - colliding with terrain results in instant fail
    vector<Box> boxRtn;
    if (octreeTerrain.intersect(rover.bounds, octreeTerrain.root, boxRtn)) {
        cout << "Crash on terrain at Y = " << rover.bounds.min().y() << endl;
        landingHasCrashed = true;
        hasLanded = true;

        rover.crash();
    }
    
    if (octreePad1.intersect(rover.bounds, octreePad1.root, boxRtn) && !hasLanded) {
        if (rover.velocity.y >= -1.5) {
            cout << "Landed on Pad 1 at Rover Y = " << rover.bounds.min().y() << "Surface at Y = " << getAltitude() << endl;
            landingHasCrashed = false;
            hasLanded = true;

            rover.pause();
        } else {
            cout << "Crashed on Pad 1 at Rover Y = " << rover.bounds.min().y() << "Surface at Y = " << getAltitude() << endl;
            landingHasCrashed = true;
            hasLanded = true;

            rover.crash();
        }
    } else if (octreePad2.intersect(rover.bounds, octreePad2.root, boxRtn)) {
        if (rover.velocity.y >= -1.5) {
            cout << "Landed on Pad 2 at Rover Y = " << rover.bounds.min().y() << "Surface at Y = " << getAltitude() << endl;
            landingHasCrashed = false;
            hasLanded = true;

            rover.pause();
        } else {
            cout << "Crashed on Pad 2 at Rover Y = " << rover.bounds.min().y() << "Surface at Y = " << getAltitude() << endl;
            landingHasCrashed = true;
            hasLanded = true;

            rover.crash();
        }
    } else if (octreePad3.intersect(rover.bounds, octreePad3.root, boxRtn)) {
        if (rover.velocity.y >= -1.5) {
            cout << "Landed on Pad 3 at Rover Y = " << rover.bounds.min().y() << "Surface at Y = " << getAltitude() << endl;
            landingHasCrashed = false;
            hasLanded = true;
            
            rover.pause();
        } else {
            cout << "Crashed on Pad 3 at Rover Y = " << rover.bounds.min().y() << "Surface at Y = " << getAltitude() << endl;
            landingHasCrashed = true;
            hasLanded = true;

            rover.crash();
        }
    }

    fps = ofToString(ofGetFrameRate(), 2);
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    
    // Very obvious background color
    ofBackground(30, 30, 60); // Dark blue background

    // Begin camera
    theCam->begin();

    // Draw the coordinate axes
    drawAxis(ofVec3f(0, 0, 0));
   
    if (terrain.getNumMeshes() > 0 && rover.object.getNumMeshes() > 0) {
        if (useWireframe) {
            ofDisableLighting();
            ofSetColor(0, 255, 0);
            terrain.drawWireframe();
            landingPad1.drawWireframe();
            landingPad2.drawWireframe();
            landingPad3.drawWireframe();
            rover.object.drawWireframe();
        }
        else if (lightingEnabled) {
            ofEnableLighting();
            ofSetColor(200, 200, 200);
            terrain.drawFaces();
            landingPad1.drawFaces();
            rover.draw();
            ofDrawArrow(rover.position, rover.position + rover.heading() * 40); // draws the rover heading
            // ofDrawArrow();
            rover.engine.draw();
            landingPad2.drawFaces();
            landingPad3.drawFaces();
            //draw arrows from the rover to the landing pads
            ofSetColor(ofColor::blue);
            ofDrawArrow(rover.position, landingPad1.getPosition());
            ofSetColor(ofColor::red);
            ofDrawArrow(rover.position, landingPad2.getPosition());
            ofSetColor(ofColor::green);
            ofDrawArrow(rover.position, landingPad3.getPosition());
            ofSetColor(ofColor::white);
            landing1light.draw();
            landing2light.draw();
            landing3light.draw();
            if (!landingHasCrashed) rover.draw();
            else rover.engine.draw();
            ofDisableLighting();
        }
        else {
            ofDisableLighting();
            ofSetColor(255, 160, 0);
            terrain.drawFaces();
            landingPad1.drawFaces();
            landingPad2.drawFaces();
            landingPad3.drawFaces();
            if (!landingHasCrashed) rover.draw();
            else rover.engine.draw();
        }
    }
    
    if (octreeLevels > 0) {
        ofNoFill();
        ofSetColor(ofColor::white);
        //octreeTerrain.draw(octreeLevels, 0);
        octreeTerrain.drawLeafNodes(octreeTerrain.root);
        octreePad1.draw(octreeLevels, 0);
        octreePad2.draw(octreeLevels, 0);
        octreePad3.draw(octreeLevels, 0);
        rover.drawBoundingBox();
        ofFill();
    }
    
    // End camera
    theCam->end();
    
    glDepthMask(false);
    ofSetColor(255);
    gui.draw();
    glDepthMask(true);
}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    // Use lowercase key (in case Shift or Caps Lock is on)
    int lowerKey = tolower(key);
    
    switch(lowerKey) {
        case 'l': {
            useWireframe = !useWireframe;
            cout << "Wireframe mode: " << (useWireframe ? "ON" : "OFF") << endl;
            break;
        }
        case 'r': {
            hasLanded = false;
            landingHasCrashed = false;
            rover.reset();
            break;
        }
        case 't': {
            hideGUI = !hideGUI;
            break;
        }
        case 'c': {
            if (camID == 1) {
                camID = 2;
                theCam = &rover3rdPersonCam;
                break;
            } else if (camID == 2) {
                camID = 3;
                cout << "Viewing First Person" << endl;
                theCam = &rover1stPersonCam;
                break;
            }
            else if (camID == 3) {
                camID = 4;
                cout << "Viewing Nearest Landing Pad:";
                
                glm::vec3 roverPos = rover.position;
                float dist3 = glm::distance(roverPos, landingPad3.getPosition());
                float dist2 = glm::distance(roverPos, landingPad2.getPosition());
                float dist1 = glm::distance(roverPos, landingPad1.getPosition());
                if (dist2 < dist3 && dist2<dist1) { // if landing 2 < landing 1 and 3
                    cout << "landing2" << endl;
                    theCam = &landing2Cam;
                }
                if (dist1 < dist2 && dist1 < dist3) { // landing 1 < landing 2 and 3
                    cout << "landing1" << endl;
                    theCam = &landing1Cam;
                }
                else{ 
                    cout << "landing3" << endl;
                    theCam = &landing3Cam; 
                }
            }
            else{ // camID == 4
                camID = 1;
                theCam = &roverTopDownCam;
                break;
            }
            break;
        }
        case OF_KEY_ESC: {
            switch(rover.isPaused()) {
                case true: {
                    rover.play();
                    break;
                }
                case false: {
                    rover.pause();
                    break;
                }
            }
        }
    }
    
    keymap[key] = true;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    keymap[key] = false;
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::initLightingAndMaterials() {

    static float ambient[] =
    { .5f, .5f, .5, 1.0f };
    static float diffuse[] =
    { 1.0f, 1.0f, 1.0f, 1.0f };

    static float position[] =
    {5.0, 5.0, 5.0, 0.0 };

    static float lmodel_ambient[] =
    { 1.0f, 1.0f, 1.0f, 1.0f };

    static float lmodel_twoside[] =
    { GL_TRUE };


    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, position);


    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
//    glEnable(GL_LIGHT1);
    glShadeModel(GL_SMOOTH);
}

void ofApp::drawAxis(ofVec3f location) {

    ofPushMatrix();
    ofTranslate(location);

    ofSetLineWidth(1.0);

    // X Axis
    ofSetColor(ofColor(255, 0, 0));
    ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
    

    // Y Axis
    ofSetColor(ofColor(0, 255, 0));
    ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

    // Z Axis
    ofSetColor(ofColor(0, 0, 255));
    ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

    ofPopMatrix();
}

void ofApp::updateCameras() {
    // Third-person camera still the same
    rover3rdPersonCam.setPosition(rover.position.x - 50, rover.position.y + 35, rover.position.z - 50);
    rover3rdPersonCam.lookAt(rover.position);

    // First-person camera (rotate with ship)
    float heightOffset = rover.object.getSceneCenter().y + 4;
    rover1stPersonCam.setPosition(rover.position.x, rover.position.y + heightOffset, rover.position.z);
    
    // Calculate forward vector based on ship's Y rotation
    float radians = ofDegToRad(rover.angle);  // convert angle to radians
    glm::vec3 forward = -glm::normalize(glm::vec3(sin(radians), 0, cos(radians)));  // forward in local Z

    glm::vec3 lookAtTarget = rover1stPersonCam.getPosition() + forward;

    rover1stPersonCam.lookAt(lookAtTarget);
    
    roverTopDownCam.setPosition(rover.position.x, rover.position.y + 100, rover.position.z);
    roverTopDownCam.lookAt(rover.position);
    
    // set landing cameras to look at the rover
    landing1Cam.lookAt(rover.position);
    landing2Cam.lookAt(rover.position);
    landing3Cam.lookAt(rover.position);
    // RoverLights
    roverLight.setPosition(rover.position.x, rover.position.y + 50, rover.position.z);
    roverHeadLamp.setPosition(rover.position);
    roverHeadLamp.lookAt(rover.position + rover.heading()*20);
}
/*
float ofApp::getAltitude() {
    Vector3 rmin = rover.bounds.min();
    Vector3 rmax = rover.bounds.max();

    // Narrow vertical probe box under the rover
    float probeDepth = FLT_MAX; // adjust based on terrain depth range
    Box probeBox(
        Vector3(rmin.x(), rmin.y() - probeDepth, rmin.z()),
        Vector3(rmax.x(), rmin.y(), rmax.z())
    );

    vector<Box> nearbyBoxes;
    octreeTerrain.intersect(probeBox, octreeTerrain.root, nearbyBoxes);

    float roverBottomY = rmin.y();
    float maxTerrainY = -FLT_MAX;

    for (const Box& b : nearbyBoxes) {
        float y = b.max().y();
        if (y <= roverBottomY && y > maxTerrainY) {
            maxTerrainY = y;
        }
    }

    if (maxTerrainY > -FLT_MAX) {
        return roverBottomY - maxTerrainY;
    }

    return -1.0f; // no terrain found below
}
*/

float ofApp::getAltitude() {
    Vector3 origin = Vector3(rover.position.x, rover.position.y, rover.position.z);
    Vector3 dir = Vector3(0, -1, 0);  // down direction

    Ray altitudeRay(origin, dir);
    TreeNode hitNode;

    if (octreePad1.intersect(altitudeRay, octreePad1.root, hitNode)) {
        float terrainY = hitNode.box.max().y();
        return rover.bounds.min().y() - terrainY;
    } else if (octreePad2.intersect(altitudeRay, octreePad2.root, hitNode)) {
        float terrainY = hitNode.box.max().y();
        return rover.bounds.min().y() - terrainY;
    } else if (octreePad3.intersect(altitudeRay, octreePad3.root, hitNode)) {
        float terrainY = hitNode.box.max().y();
        return rover.bounds.min().y() - terrainY;
    } else {
        return -1.0f; // no intersection found
    }
}


void ofApp::loadModelAtPosition(ofxAssimpModelLoader &model, const std::string &path, const ofVec3f &pos, ofMesh &worldMeshOut) {
    // Load and position model
    model.load(path);
    model.setScaleNormalization(false);
    model.setPosition(pos.x, pos.y, pos.z);

    // Get full transform matrix to apply to vertices
    ofMatrix4x4 transform = model.getModelMatrix();

    // Build a world-transformed mesh
    worldMeshOut.clear();
    for (int i = 0; i < model.getMeshCount(); i++) {
        ofMesh local = model.getMesh(i);
        for (int j = 0; j < local.getNumVertices(); j++) {
            ofVec3f v = local.getVertex(j);
            v = v * transform;  // apply transformation
            worldMeshOut.addVertex(v);
        }
        for (int j = 0; j < local.getNumIndices(); j++) {
            worldMeshOut.addIndex(local.getIndex(j));
        }
    }
}
