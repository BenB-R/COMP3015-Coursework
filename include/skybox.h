#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../include/shader.h"

class Skybox
{
public:
    Skybox(const std::vector<std::string>& faces);
    ~Skybox();

    void LoadTextures();
    void Draw(const glm::mat4& view, const glm::mat4& projection);

private:
    Shader skyboxShader;
    unsigned int cubemapTexture;
    unsigned int VAO, VBO;
    std::vector<std::string> faces;

    void setupSkybox();
};

#endif