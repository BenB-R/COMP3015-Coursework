// Essential headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/gtc/random.hpp>
#include <vector>

// GLM for mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Internal headers
#include "../include/shader.h"
#include "../include/model.h"
#include "../include/camera.h"
#include "../include/skybox.h"

// Callback function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);

// Settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

const int NUM_TREES = 100; // Example number, adjust as needed
const float EXCLUSION_RADIUS = 10.0f; // Trees won't spawn within this radius from the center
const float MAX_SPAWN_RADIUS = 100.0f; // Maximum distance from the center where trees can spawn
const float DENSITY_INCREASE_END_RADIUS = 50.0f; // Distance at which density stops increasing and becomes constant

// Function to check if a position is within the spawning criteria
bool isValidSpawnPosition(const glm::vec3& position, float exclusionRadius, float maxSpawnRadius, float densityIncreaseEndRadius) {
    float distanceFromCenter = glm::length(position); // Calculate distance from the center (0, 0, 0)
    if (distanceFromCenter < exclusionRadius) return false; // Inside exclusion zone
    if (distanceFromCenter > maxSpawnRadius) return false; // Beyond maximum spawn distance
    return true;
}

// Function to generate a position with a probability that increases with distance from center, up to a point
glm::vec3 generatePositionWithIncreasingDensity(float exclusionRadius, float maxSpawnRadius, float densityIncreaseEndRadius) {
    glm::vec3 position;
    do {
        float randX = glm::linearRand(-maxSpawnRadius, maxSpawnRadius);
        float randZ = glm::linearRand(-maxSpawnRadius, maxSpawnRadius);
        position = glm::vec3(randX, 0.0f, randZ);
    } while (!isValidSpawnPosition(position, exclusionRadius, maxSpawnRadius, densityIncreaseEndRadius));
    return position;
}

std::vector<glm::vec3> generateTreePositions(int numTrees, float exclusionRadius, float maxSpawnRadius, float densityIncreaseEndRadius) {
    std::vector<glm::vec3> positions;
    for (int i = 0; i < numTrees; ++i) {
        glm::vec3 position = generatePositionWithIncreasingDensity(exclusionRadius, maxSpawnRadius, densityIncreaseEndRadius);
        positions.push_back(position);
    }
    return positions;
}

// Object Initialisation
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
bool firstMouse = true;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

std::vector<glm::vec3> treePositions; // Array to hold tree positions

std::vector<std::string> faces = {
    "assets/textures/skybox/right.png",
    "assets/textures/skybox/left.png",
    "assets/textures/skybox/top.png",
    "assets/textures/skybox/bottom.png",
    "assets/textures/skybox/front.png",
    "assets/textures/skybox/back.png"
}; //skybox faces

int main()
{
#pragma region setup
    // Initialize and configure GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // GLFW window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Forest Scene", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouseCallback);

    // Load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
#pragma endregion setup

#pragma region shaders
    Shader basicShader("shaders/basic_vert.vs", "shaders/basic_frag.fs"); // General objects
#pragma endregion

#pragma region models
    Model tree1("assets/models/tree1/Tree1.obj");
    Skybox skybox(faces);
#pragma endregion

#pragma region floor
    unsigned int planeTexture;
    glGenTextures(1, &planeTexture);
    glBindTexture(GL_TEXTURE_2D, planeTexture);
    // Set texture wrapping/filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load and generate the texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load("assets/textures/grass.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // Step 1: Plane Vertex Data
    float planeVertices[] = {
        // positions            // normals (optional for lighting) // texture coords (optional)
        500.0f, 0.0f, 500.0f,    0.0f, 1.0f, 0.0f,                 100.0f, 100.0f,   // top right
        -500.0f, 0.0f, 500.0f,   0.0f, 1.0f, 0.0f,                 0.0f, 100.0f,   // top left
        -500.0f, 0.0f, -500.0f,  0.0f, 1.0f, 0.0f,                 0.0f, 0.0f,   // bottom left
        500.0f, 0.0f, -500.0f,   0.0f, 1.0f, 0.0f,                 100.0f, 0.0f    // bottom right
    };

    unsigned int planeIndices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    // Step 2: Setup plane VAO and VBO
    unsigned int planeVAO, planeVBO, planeEBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glGenBuffers(1, &planeEBO);

    glBindVertexArray(planeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), &planeIndices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute (if you're using lighting)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture coord attribute (if you're using textures)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Unbind the VAO
    glBindVertexArray(0);
    
#pragma endregion

    treePositions = generateTreePositions(NUM_TREES, EXCLUSION_RADIUS, MAX_SPAWN_RADIUS, DENSITY_INCREASE_END_RADIUS);
    glm::vec3 firePos(15.0f, 5.0f, 3.0f); // Light position at the center of the scene

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate delta time for smooth camera movement
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);

        // Rendering commands here
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Camera/View transformation
        // Set up camera and projection matrices (common for both crystals and cave)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 500.0f);
        glm::mat4 view = camera.getViewMatrix();

        skybox.Draw(view, projection);

        // Activate shader
        basicShader.use();

        basicShader.setMat4("projection", projection);
        basicShader.setMat4("view", view);

        basicShader.setVec3("lightPos", firePos);
        basicShader.setVec3("viewPos", camera.Position);

        // Render Tree1
        for (const auto& position : treePositions) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, position); // Use the random position
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f)); // Keep scale uniform
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate if needed
            basicShader.setMat4("model", model);
            tree1.Draw(basicShader, model); // Draw each tree
        }
#pragma region plane
        glActiveTexture(GL_TEXTURE0); // Activate the texture unit before binding
        glBindTexture(GL_TEXTURE_2D, planeTexture); // Bind the plane's texture
        basicShader.setInt("texture_diffuse1", 0); // Set the texture uniform if required
        glBindVertexArray(planeVAO);
        glm::mat4 model = glm::mat4(1.0f);
        // Adjust model matrix for the plane if necessary
        basicShader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
#pragma endregion

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // GLFW: terminate, clearing all previously allocated GLFW resources
    glfwTerminate();
    return 0;
}

// Callback function for adjusting the viewport size
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);
}

// Process all input: query GLFW whether relevant keys are pressed/released this frame
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    bool isSprinting = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(FORWARD, deltaTime, isSprinting);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(BACKWARD, deltaTime, isSprinting);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(LEFT, deltaTime, isSprinting);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(RIGHT, deltaTime, isSprinting);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.processKeyboard(UP, deltaTime, isSprinting);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.processKeyboard(DOWN, deltaTime, isSprinting);
}