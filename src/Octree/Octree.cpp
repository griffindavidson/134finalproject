#include "Octree.h"

// Draw a box from a "Box" class
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

// Return a Mesh Bounding Box for the entire Mesh
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
    return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}

int Octree::getMeshPointsInBox(const ofMesh & mesh, const vector<int>& points, Box & box, vector<int> & pointsRtn) {
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

int Octree::getMeshFacesInBox(const ofMesh & mesh, const vector<int>& faces, Box & box, vector<int> & facesRtn) {
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
        if (box.inside(p, 3)) {
            count++;
            facesRtn.push_back(faces[i]);
        }
    }
    return count;
}

void Octree::subDivideBox8(const Box &box, vector<Box> & boxList) {
    Vector3 min = box.parameters[0];
    Vector3 max = box.parameters[1];
    Vector3 size = max - min;
    Vector3 center = size / 2 + min;
    float xdist = size.x() / 2;
    float ydist = size.y() / 2;
    float zdist = size.z() / 2;
    Vector3 h = Vector3(0, ydist, 0);

    Box b[8];
    b[0] = Box(min, center);
    b[1] = Box(b[0].min() + Vector3(xdist, 0, 0), b[0].max() + Vector3(xdist, 0, 0));
    b[2] = Box(b[1].min() + Vector3(0, 0, zdist), b[1].max() + Vector3(0, 0, zdist));
    b[3] = Box(b[2].min() + Vector3(-xdist, 0, 0), b[2].max() + Vector3(-xdist, 0, 0));

    boxList.clear();
    for (int i = 0; i < 4; i++) boxList.push_back(b[i]);
    for (int i = 4; i < 8; i++) {
        b[i] = Box(b[i - 4].min() + h, b[i - 4].max() + h);
        boxList.push_back(b[i]);
    }
}

void Octree::create(const ofMesh & geo, int numLevels) {
    mesh = geo;
    int level = 0;
    root.box = meshBounds(mesh);

    if (!bUseFaces) {
        for (int i = 0; i < mesh.getNumVertices(); i++) {
            root.points.push_back(i);
        }
    } else {
        int numFaces = mesh.hasIndices() ? mesh.getNumIndices() / 3 : mesh.getNumVertices() / 3;
        for (int i = 0; i < numFaces; i++) {
            root.points.push_back(i);
        }
    }

    level++;
    subdivide(mesh, root, numLevels, level);
}

void Octree::subdivide(const ofMesh & mesh, TreeNode & node, int numLevels, int level) {
    if (level >= numLevels) return;

    vector<Box> childBoxes;
    subDivideBox8(node.box, childBoxes);
    const vector<int>& faces = node.points;

    for (const Box &childBox : childBoxes) {
        vector<int> childFaces;
        int count = getMeshFacesInBox(mesh, faces, const_cast<Box&>(childBox), childFaces);

        if (count > 0) {
            TreeNode childNode;
            childNode.box = childBox;
            childNode.points = childFaces;
            node.children.push_back(childNode);

            if (childFaces.size() > 1) {
                subdivide(mesh, node.children.back(), numLevels, level + 1);
            }
        }
    }
}

bool Octree::intersect(const Ray &ray, const TreeNode &node, TreeNode &nodeRtn) {
    if (!node.box.intersect(ray, 0.0f, 1000.0f)) return false;
    if (node.children.empty()) {
        nodeRtn = node;
        return true;
    }
    for (const TreeNode &child : node.children) {
        if (intersect(ray, child, nodeRtn)) return true;
    }
    return false;
}

bool Octree::intersect(const Box &box, TreeNode &node, vector<Box> &boxListRtn) {
    if (!node.box.overlap(box)) return false;
    bool intersected = false;
    if (node.children.empty()) {
        boxListRtn.push_back(node.box);
        return true;
    }
    for (TreeNode &child : node.children) {
        if (intersect(box, child, boxListRtn)) intersected = true;
    }
    return intersected;
}

void Octree::draw(TreeNode & node, int numLevels, int level) {
    if (level >= numLevels) return;
    ofSetColor(colors[level % colors.size()]);
    drawBox(node.box);
    for (TreeNode &child: node.children) {
        draw(child, numLevels, level + 1);
    }
    ofSetColor(ofColor::white);
}

void Octree::drawLeafNodes(TreeNode & node) {
    if (node.children.empty()) {
        ofSetColor(ofColor::green);
        drawBox(node.box);
    } else {
        for (TreeNode &child: node.children) {
            drawLeafNodes(child);
        }
    }
}
