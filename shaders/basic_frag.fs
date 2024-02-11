#version 330 core
out vec4 FragColor;

in vec2 TexCoords; // Received from vertex shader

uniform sampler2D texture_diffuse1; // Texture uniform

void main() {
    FragColor = texture(texture_diffuse1, TexCoords);
}
