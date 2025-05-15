#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "DynamicObject.h"
#include "ofxAssimpModelLoader.h"
#include "Octree/Octree.h"

class ofApp : public ofBaseApp{

	public:
		void setup() override;
		void update() override;
		void draw() override;
		void exit() override;

		void keyPressed(int key) override;
		void keyReleased(int key) override;
		void mouseMoved(int x, int y ) override;
		void mouseDragged(int x, int y, int button) override;
		void mousePressed(int x, int y, int button) override;
		void mouseReleased(int x, int y, int button) override;
		void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
		void mouseEntered(int x, int y) override;
		void mouseExited(int x, int y) override;
		void windowResized(int w, int h) override;
		void dragEvent(ofDragInfo dragInfo) override;
		void gotMessage(ofMessage msg) override;
    
        float getAltitude();
    
        map<int, bool> keymap;
    
        void initLightingAndMaterials();
        void drawAxis(ofVec3f location);
    
        ofxAssimpModelLoader terrain;
        Octree octreeTerrain;
    
        Ship rover = Ship();
    
        ofxAssimpModelLoader landingPad1;
        Octree octreePad1;
    
        ofxAssimpModelLoader landingPad2;
        Octree octreePad2;
        
        ofxAssimpModelLoader landingPad3;
        Octree octreePad3;
    
        void updateCameras();
        
        ofEasyCam* theCam;
        int camID = 2;
        ofEasyCam cam; // id
        ofEasyCam rover3rdPersonCam; //id 1
        ofEasyCam rover1stPersonCam; // id 2
        ofEasyCam roverTopDownCam; // id 3
    
        ofEasyCam landing1Cam; // id 4
        ofEasyCam landing2Cam; // id 4 // calculate and use closest camera as id 4
        ofEasyCam landing3Cam; // id 4
    
        bool lightingEnabled = true;
        bool useWireframe = false;
        bool showOriginCube = false;
        bool showDebugSphere = false;
    
        bool hideGUI = false;
        ofxPanel gui;
        ofParameter<int> octreeLevels;
        ofxLabel fps;
        ofxLabel altitudeLabel;
        ofxLabel thrust;
    
        ofxLabel moveLabel;
        ofxLabel rotateLabel;
        ofxLabel toggleCam;
        ofxLabel pauseLabel;
        ofxLabel velocityLabel;
    
        bool hasLanded = false; // boolean saving if the rover has landed yet or not
        bool landingHasCrashed = false; // landing type: true = crashed; false = successful
};
