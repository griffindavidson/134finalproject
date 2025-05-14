#version 120

varying vec4 vColor;

void main() {
    // Soft circular mask (alpha fades near edge)
    float dist = length(gl_PointCoord - vec2(0.5));
    if (dist > 0.5) discard;

    gl_FragColor = vColor;
}
