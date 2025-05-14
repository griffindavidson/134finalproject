#version 120

uniform mat4 modelViewProjectionMatrix;

attribute vec4 position;
attribute vec4 color;

varying vec4 vColor;

void main() {
    vColor = color;
    gl_Position = modelViewProjectionMatrix * position;
    gl_PointSize = 15.0;
}
