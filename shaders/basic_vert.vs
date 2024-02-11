#version 330 core
layout (location = 0) in vec3 aPos;   // Vertex position
layout (location = 1) in vec2 aTexCoords; // Texture coordinates
layout (location = 2) in vec3 aNormal; // Vertex normals (not used here but typically needed)

out vec2 TexCoords; // Pass texture coordinates to fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
