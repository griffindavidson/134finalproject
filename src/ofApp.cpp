#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // Basic setup
    ofSetEscapeQuitsApp(false);
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofEnableSmoothing();
    ofEnableDepthTest();
    ofSetBackgroundColor(30, 30, 30); // Dark gray background
    
    // Camera setup
    cam.setDistance(50);  // Start much further back
    cam.setNearClip(0.1);
    cam.setFarClip(1000);
    cam.setFov(60);
    cam.enableMouseInput();
    cam.setPosition(10, 10, 10);
    
    theCam = &rover3rdPersonCam;
    
    // Initialize lighting
    initLightingAndMaterials();
    
    moonTerrain.load("geo/moon-houdini.obj");
    moonTerrain.setScaleNormalization(false);
    moonTerrain.setPosition(0, 0, 0);
    octree.bUseFaces = true;
    octree.create(moonTerrain.getMesh(0), 20);
    
    rover.object.setPosition(0, 10, 0);
    rover.object.setScale(0.05, 0.05, 0.05);
    rover.setGlobalForce(ofVec3f(0, -1.625, 0));
    
    rover3rdPersonCam.setPosition(-50, 45, -50);
    rover3rdPersonCam.lookAt(rover.position);
    rover3rdPersonCam.setNearClip(0.1);
    rover3rdPersonCam.setFarClip(1000);
    rover3rdPersonCam.setFov(60);
    rover3rdPersonCam.disableMouseInput();
    
    float roverMiddleHeight = rover.object.getSceneCenter().y;
    rover1stPersonCam.setPosition(0, roverMiddleHeight, 0);
    rover1stPersonCam.lookAt(glm::vec3(1, 0, 1)); // random direction for now, make it look at normal
    rover1stPersonCam.setNearClip(7.25);
    rover1stPersonCam.setFarClip(1000);
    rover1stPersonCam.setFov(50);
    rover1stPersonCam.disableMouseInput();
    
    roverTopDownCam.setPosition(0, 110, 0);
    roverTopDownCam.lookAt(rover.position);
    roverTopDownCam.setNearClip(0);
    roverTopDownCam.setFarClip(1000);
    roverTopDownCam.setFov(65.5);
    roverTopDownCam.disableMouseInput();
    
    gui.setup();
    //gui.add(octreeLevels.set("Octree Levels", 0, 0, 20));
    gui.add(fps.setup("FPS", ""));
    gui.add(altitudeLabel.setup("Altitude", ""));
    gui.add(thrust.setup("Thrust", ""));
    
    //cout << "Root box: (" << ofToString(octree.root.box.min().x()) << "," << ofToString(octree.root.box.min().y()) << ", " << ofToString(octree.root.box.min().z()) << ")" << " to " << "(" << ofToString(octree.root.box.max().x()) << "," << ofToString(octree.root.box.max().y()) << ", " << ofToString(octree.root.box.max().z()) << ")" << endl;
    //cout << "Number of children: " << ofToString(octree.root.children.size()) << endl;

}

//--------------------------------------------------------------
void ofApp::update(){
    updateCameras();
    rover.update();
    rover.engine.setRate(0);
    
    if (!rover.isPaused()) {
        if (keymap[' '] && rover.thrust > 0.0f) {
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

    
    if (rover.thrust <= 0) rover.thrust = 0.0f;
    thrust = ofToString(rover.thrust / 5.0f * 100.0f) + "%";
    
    glm::vec3 forwardDir, rightDir;

    if (theCam == &roverTopDownCam) {
        // Top-down: "forward" is the direction the camera is pointing, flattened to XZ
        forwardDir = glm::normalize(theCam->getLookAtDir());
        forwardDir.y = 0;
        forwardDir = glm::normalize(forwardDir);
        rightDir = glm::normalize(glm::cross(forwardDir, glm::vec3(0,1,0)));
    } else {
        // 1st-person or 3rd-person: Use camera direction
        forwardDir = glm::normalize(theCam->getLookAtDir());
        forwardDir.y = 0;
        forwardDir = glm::normalize(forwardDir);
        rightDir = glm::normalize(glm::cross(forwardDir, glm::vec3(0,1,0)));
    }

    // Movement input
    float moveSpeed = 2;

    if (keymap['w']) {
        rover.applyForce(ofVec3f(forwardDir.x, forwardDir.y, forwardDir.z) * moveSpeed);
    }
    if (keymap['s']) {
        rover.applyForce(ofVec3f(-forwardDir.x, -forwardDir.y, -forwardDir.z) * moveSpeed);
    }
    if (keymap['a']) {
        rover.applyForce(ofVec3f(-rightDir.x, -rightDir.y, -rightDir.z) * moveSpeed);
    }
    if (keymap['d']) {
        rover.applyForce(ofVec3f(rightDir.x, rightDir.y, rightDir.z) * moveSpeed);
    }


    if (keymap['q']) { // ROTATE LEFT
        rover.applyRotationForce(15);
    }
    
    if (keymap['e']) { // ROTATE RIGHT
        rover.applyRotationForce(-15);
    }
    
    altitudeLabel = ofToString(getAltitude()) + "m";
    
    // COLLISION
    vector<Box> leafBoxes;
    octree.collectLeafBoxes(octree.root, leafBoxes);

    // Get rover bounds (bottom Y and XZ min/max)
    Vector3 rmin = rover.bounds.min();
    Vector3 rmax = rover.bounds.max();
    float roverBottomY = rmin.y();
    ofVec2f roverMinXZ(rmin.x(), rmin.z());
    ofVec2f roverMaxXZ(rmax.x(), rmax.z());

    // Loop through octree leaf boxes
    float epsilon = 0.5f;
    for (const Box& leafBox : leafBoxes) {
        Vector3 tmin = leafBox.min();
        Vector3 tmax = leafBox.max();

        float terrainTopY = tmax.y();
        ofVec2f terrainMinXZ(tmin.x(), tmin.z());
        ofVec2f terrainMaxXZ(tmax.x(), tmax.z());

        // Check if rover is vertically close to the top of the terrain box
        if (abs(roverBottomY - terrainTopY) < epsilon && rover.velocity.y <= 0) {
            bool xOverlap = roverMaxXZ.x >= terrainMinXZ.x && roverMinXZ.x <= terrainMaxXZ.x;
            bool zOverlap = roverMaxXZ.y >= terrainMinXZ.y && roverMinXZ.y <= terrainMaxXZ.y;

            if (xOverlap && zOverlap) {
                cout << "Collision with terrain at Y = " << terrainTopY << endl;

                float roverHeight = rmax.y() - rmin.y();
                rover.position.y = terrainTopY; // or + roverHeight/2.0 if needed
                rover.velocity.y = 0;
                break;
            }
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
   
    if (moonTerrain.getNumMeshes() > 0 && rover.object.getNumMeshes() > 0) {
        if (useWireframe) {
            ofDisableLighting();
            ofSetColor(0, 255, 0);
            moonTerrain.drawWireframe();
            rover.object.drawWireframe();
        }
        else if (lightingEnabled) {
            ofEnableLighting();
            ofSetColor(200, 200, 200);
            moonTerrain.drawFaces();
            rover.draw();
            ofDisableLighting(); // Clean up after
        }
        else {
            ofDisableLighting();
            ofSetColor(255, 160, 0);
            moonTerrain.drawFaces();
            rover.draw();
        }
    }
    
    if (octreeLevels > 0) {
        ofNoFill();
        ofSetColor(ofColor::white);
        octree.draw(octreeLevels, 0);
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
            // Reset camera
            cam.reset();
            cam.setDistance(50);
            cout << "Camera reset" << endl;
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
                theCam = &rover1stPersonCam;
                break;
            } else {
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

float ofApp::getVerticalDistanceToTerrain(ofVec3f roverPos, const ofMesh& terrainMesh) {
    float minXZDist = std::numeric_limits<float>::max();
    float terrainYAtClosestXZ = 0;

    for (int i = 0; i < terrainMesh.getNumVertices(); ++i) {
        ofVec3f v = terrainMesh.getVertex(i);

        // Distance in the XZ plane only
        float xzDist = ofVec2f(v.x, v.z).distance(ofVec2f(roverPos.x, roverPos.z));

        if (xzDist < minXZDist) {
            minXZDist = xzDist;
            terrainYAtClosestXZ = v.y;
        }
    }

    return roverPos.y - terrainYAtClosestXZ;
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
    glm::vec3 forward = glm::normalize(glm::vec3(sin(radians), 0, cos(radians)));  // forward in local Z

    glm::vec3 lookAtTarget = rover1stPersonCam.getPosition() + forward;

    rover1stPersonCam.lookAt(lookAtTarget);
    
    roverTopDownCam.setPosition(rover.position.x, rover.position.y + 100, rover.position.z);
    roverTopDownCam.lookAt(rover.position);
}

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
    octree.intersect(probeBox, octree.root, nearbyBoxes);

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
