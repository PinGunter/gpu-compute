#version 430 core

out vec4 FragColor;

in vec2 TexCoords;

// La textura que leeremos (la misma que el compute shader escribió)
uniform sampler2D screenTexture;

void main() {
    FragColor = texture(screenTexture, TexCoords);
}