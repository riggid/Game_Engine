#version 330 core
out vec4 FragColor;

uniform vec3 laserColor;

void main() {
    FragColor = vec4(laserColor, 1.0);
}
