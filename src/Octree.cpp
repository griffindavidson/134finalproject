
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Simple Octree Implementation 11/10/2020
// 
//  Copyright (c) by Kevin M. Smith
//  Copying or use without permission is prohibited by law. 
//


#include "Octree.h"
 


//draw a box from a "Box" class  
//
void Octree::drawBox(const Box &box) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
	float w = size.x();
	float h = size.y();
	float d = size.z();
	ofDrawBox(p, w, h, d);
}

// return a Mesh Bounding Box for the entire Mesh
//
Box Octree::meshBounds(const ofMesh & mesh) {
	int n = mesh.getNumVertices();
	ofVec3f v = mesh.getVertex(0);
	ofVec3f max = v;
	ofVec3f min = v;
	for (int i = 1; i < n; i++) {
		ofVec3f v = mesh.getVertex(i);

		if (v.x > max.x) max.x = v.x;
		else if (v.x < min.x) min.x = v.x;

		if (v.y > max.y) max.y = v.y;
		else if (v.y < min.y) min.y = v.y;

		if (v.z > max.z) max.z = v.z;
		else if (v.z < min.z) min.z = v.z;
	}
	cout << "vertices: " << n << endl;
//	cout << "min: " << min << "max: " << max << endl;
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}

// getMeshPointsInBox:  return an array of indices to points in mesh that are contained 
//                      inside the Box.  Return count of points found;
//
int Octree::getMeshPointsInBox(const ofMesh & mesh, const vector<int>& points,
	Box & box, vector<int> & pointsRtn)
{
	int count = 0;
	for (int i = 0; i < points.size(); i++) {
		ofVec3f v = mesh.getVertex(points[i]);
		if (box.inside(Vector3(v.x, v.y, v.z))) {
			count++;
			pointsRtn.push_back(points[i]);
		}
	}
	return count;
}

// don't worry about faces for this assignment
// getMeshFacesInBox:  return an array of indices to Faces in mesh that are contained 
//                      inside the Box.  Return count of faces found;
//
int Octree::getMeshFacesInBox(const ofMesh & mesh, const vector<int>& faces,
	Box & box, vector<int> & facesRtn)
{
	int count = 0;
	for (int i = 0; i < faces.size(); i++) {
		ofMeshFace face = mesh.getFace(faces[i]);
		ofVec3f v[3];
		v[0] = face.getVertex(0);
		v[1] = face.getVertex(1);
		v[2] = face.getVertex(2);
		Vector3 p[3];
		p[0] = Vector3(v[0].x, v[0].y, v[0].z);
		p[1] = Vector3(v[1].x, v[1].y, v[1].z);
		p[2] = Vector3(v[2].x, v[2].y, v[2].z);
		if (box.inside(p,3)) {
			count++;
			facesRtn.push_back(faces[i]);
		}
	}
	return count;
}

//  Subdivide a Box into eight(8) equal size boxes, return them in boxList;
//
void Octree::subDivideBox8(const Box &box, vector<Box> & boxList) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	float xdist = (max.x() - min.x()) / 2;
	float ydist = (max.y() - min.y()) / 2;
	float zdist = (max.z() - min.z()) / 2;
	Vector3 h = Vector3(0, ydist, 0);

	//  generate ground floor
	//
	Box b[8];
	b[0] = Box(min, center);
	b[1] = Box(b[0].min() + Vector3(xdist, 0, 0), b[0].max() + Vector3(xdist, 0, 0));
	b[2] = Box(b[1].min() + Vector3(0, 0, zdist), b[1].max() + Vector3(0, 0, zdist));
	b[3] = Box(b[2].min() + Vector3(-xdist, 0, 0), b[2].max() + Vector3(-xdist, 0, 0));

	boxList.clear();
	for (int i = 0; i < 4; i++)
		boxList.push_back(b[i]);

	// generate second story
	//
	for (int i = 4; i < 8; i++) {
		b[i] = Box(b[i - 4].min() + h, b[i - 4].max() + h);
		boxList.push_back(b[i]);
	}
}

void Octree::create(const ofMesh & geo, int numLevels) {
	// initialize octree structure
	//
	mesh = geo;
	int level = 0;
	root.box = meshBounds(mesh);
	if (!bUseFaces) {
		for (int i = 0; i < mesh.getNumVertices(); i++) {
			root.points.push_back(i);
		}
	}
	else {
		// need to load face vertices here
		//
	}

	// recursively buid octree
	//
	level++;
    subdivide(mesh, root, numLevels, level);
	cout << "numLeaf:" << numLeaf << endl;
	// make color gradient for visuals
	for (int i = 0;i < numLevels;i++) {
		float hue = ofMap(i, 0, numLevels - 1, 0, 255);
		gradient.push_back(ofColor::fromHsb(hue, 200, 255));
	}
}


//
// subdivide:  recursive function to perform octree subdivision on a mesh
//
//  subdivide(node) algorithm:
//     1) subdivide box in node into 8 equal side boxes - see helper function subDivideBox8().
//     2) For each child box
//            sort point data into each box  (see helper function getMeshFacesInBox())
//        if a child box contains at least 1 point
//            add child to tree
//            if child is not a leaf node (contains more than 1 point)
//               recursively call subdivide(child)
//         
//      
  

void Octree::subdivide(const ofMesh & mesh, TreeNode & node, int numLevels, int level) {
	if (level >= numLevels) return;

	// subdvide algorithm implemented here
	
	// create children boxes locally within function
	vector<Box> newBoxes;
	
	// subdivide the current node box into new boxes for the children
	subDivideBox8(node.box, newBoxes);
	// cout << "subdivide boxes size: " << newBoxes.size()<<endl;
	// we are about to create a new level so we increment the level parameter
	// we don't do it in the loop, otherwise the right side of the tree would be shorter 
	// because the level would increment for each child (<= eight times), which is not what it represents
	level++;
	// cout << "numLevels:" << numLevels << "level: " << level << endl;
	for (int i = 0; i < newBoxes.size();i++) {
		vector<int> pointsInBox;
		// not sure if "node.points" is correct for that parameter
		if (getMeshPointsInBox(mesh, node.points, newBoxes[i],pointsInBox) > 0) {
			TreeNode child;
			child.box = newBoxes[i];
			child.points = pointsInBox;
			if (pointsInBox.size() == 1) { numLeaf++; } // debug line
			subdivide(mesh, child, numLevels, level);
			node.children.push_back(child); // order here matters! must subdivide the children BEFORE they are added to list.
		}
	}

}

// Implement functions below for Homework project
//
// 3). Implement  Octree::intersect(const Ray &ray, ...) using recursion.  
// Use the Octree to test Ray selection with the Mouse by selecting points in the mesh.  
// If you select a point, draw a color sphere around it.  
// (Hint: use the ray-box intersection technique you did in the lab using 
// the box.cc/box.h code in your intersect() method.


bool Octree::intersect(const Ray &ray, const TreeNode & node, TreeNode & nodeRtn) {
	if (node.points.size() == 1) { // base case is we hit a leaf node
		nodeRtn = node; // return the node of the hit in "nodeRtn"
		return true; 
	} 
	bool intersects = false; // start as false and mark true when it hits
	for (int i = 0; i < node.children.size();i++) { // iterate children of current node
		if (node.children[i].box.intersect(ray, 0, FLT_MAX)) { // if ray intersect child box
			intersects = intersect(ray, node.children[i], nodeRtn); // check that nodes children for intersection
			if (intersects) { return true; } // this line shortcuts if the intersection is found in the first 3 child nodes
		}
	}
	return intersects;
}

bool Octree::intersect(const Box &box, TreeNode & node, vector<Box> & boxListRtn) {
	if (node.points.size() == 1) { // leaf node base case
		boxListRtn.push_back(node.box); // only leaf nodes are returned
		return true;
	}
	bool intersects = false; // start as false and mark true when hit in the base case
	for (int i = 0;i < node.children.size();i++) { // loop children
		if (node.children[i].box.overlap(box)) { // if given box overlaps children
			intersects = intersect(box, node.children[i], boxListRtn); // check children for overlap
		}
	}
	return intersects;
}
// it is important to pass the address of the node instead of the node itself 
// otherwise it gets copied many times in memory
//					(here->)
void Octree::draw(TreeNode & node, int numLevels, int level) {
	if (level >= numLevels) return;
	ofSetColor(gradient[level]);
	drawBox(node.box);
	level++;
	for (int i = 0; i < node.children.size(); i++) {
		draw(node.children[i], numLevels, level);
	}
}

// Optional
//
void Octree::drawLeafNodes(TreeNode & node) {


}




