#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Vertex shader source code
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 transform;
void main()
{
    gl_Position = transform * vec4(aPos, 1.0);
}
)";

// Fragment shader source code
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(0.4, 0.8, 0.6, 1.0);
}
)";

// Function to compile shader and check for errors
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Check for compile errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return shader;
}

// Function to create shader program
GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Clean up shaders as they're linked now
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Helper function to initialize identity matrix
void identityMatrix(float* matrix) {
    for (int i = 0; i < 16; i++) {
        matrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;  // Set diagonal to 1, others to 0
    }
}

// Helper function to create a rotation matrix
void createRotationMatrix(float angle, float* matrix) {
    identityMatrix(matrix);  // Start with identity matrix
    float cosAngle = cosf(angle * M_PI / 180.0f);
    float sinAngle = sinf(angle * M_PI / 180.0f);

    matrix[0] = cosAngle;
    matrix[1] = -sinAngle;
    matrix[4] = sinAngle;
    matrix[5] = cosAngle;
}

// Helper function to create a translation matrix
void createTranslationMatrix(float x, float y, float* matrix) {
    identityMatrix(matrix);  // Start with identity matrix
    matrix[3] = x;  // Translate on x-axis
    matrix[7] = y;  // Translate on y-axis
}

// Helper function to create a scaling matrix
void createScalingMatrix(float scaleX, float scaleY, float* matrix) {
    identityMatrix(matrix);  // Start with identity matrix
    matrix[0] = scaleX;  // Scale on x-axis
    matrix[5] = scaleY;  // Scale on y-axis
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create GLFW window and context
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Controllable Triangle", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    float vertices[] = {
        0.0f,  0.25f, 0.0f,  // Top vertex (smaller)
       -0.25f, -0.25f, 0.0f,  // Bottom-left vertex
        0.25f, -0.25f, 0.0f   // Bottom-right vertex
    };


    // Generate VAO and VBO
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Compile and link shader program
    GLuint shaderProgram = createShaderProgram();
    glUseProgram(shaderProgram);

// Define initial translation values to place the triangle at a visible position near the bottom-left
    float translationX = -1.0f; 
    float translationY = -0.75f;  

    float rotationAngle = 0.0f;
    bool isJumping = false;
    float jumpHeight = 0.0f;  // Maximum height of the jump
    float jumpSpeed = 0.1f;   // Speed of jumping
    float jumpStartTime = 0.0f; // Time when jump started
    float jumpDuration = 1.0f;  // Duration of jump (in seconds)

    // Time tracking
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate deltaTime
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Control translation (left and right only)
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            translationX -= 0.01f;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            translationX += 0.01f;

        // Remove vertical movement control (no translationY changes)


        // Start jump when space is pressed
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isJumping) {
            isJumping = true;
            jumpStartTime = currentFrame;
        }

        // Handle jumping logic
        if (isJumping) {
            float jumpProgress = (currentFrame - jumpStartTime) / jumpDuration;
            if (jumpProgress < 1.0f) {
                jumpHeight = sinf(jumpProgress * M_PI) * 0.5f;  // Adjust jump height
            }
            else {
                jumpHeight = 0.0f;
                isJumping = false;  // End jump
            }
        }

        // Create transformation matrix
        float transform[16];
        identityMatrix(transform);
        createTranslationMatrix(translationX, translationY + jumpHeight, transform);
        float rotation[16];
        createRotationMatrix(rotationAngle, rotation);
        float finalTransform[16];
        for (int i = 0; i < 16; ++i)
            finalTransform[i] = transform[i];
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_TRUE, finalTransform);

        // Clear screen and render triangle
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup and terminate
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
