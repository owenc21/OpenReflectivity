#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>

#include "decoder.hpp"

// Shader sources
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in float aIntensity;
    out float intensity;
    uniform mat4 transform;
    void main()
    {
        gl_Position = transform * vec4(aPos, 0.0, 1.0);
        intensity = aIntensity;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in float intensity;
    out vec4 FragColor;
    vec3 colorMap(float t) {
        if (t < 0.25) return mix(vec3(0,0,1), vec3(0,1,0), t*4);
        if (t < 0.5) return mix(vec3(0,1,0), vec3(1,1,0), (t-0.25)*4);
        if (t < 0.75) return mix(vec3(1,1,0), vec3(1,0,0), (t-0.5)*4);
        return mix(vec3(1,0,0), vec3(1,0,1), (t-0.75)*4);
    }
    void main()
    {
        FragColor = vec4(colorMap(intensity), 1.0);
    }
)";

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation error: " << infoLog << std::endl;
    }
    return shader;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Radar Plot", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    // Compile and link shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Read radar data from decoder
    std::vector<std::vector<float>> rads;
	std::string file_name = "archives/KDIX20240623_234538_V06";
	archive_file file;
	int decode = Decoder::DecodeArchive(file_name, true, file); 
    for(std::shared_ptr<radial_data> radial : file.scan_elevations[1]->radials){
        rads.push_back(radial->ref->data);
    }

    // Process radar data into vertices
    // TODO: Adjust these values... highly unoptimized
    std::vector<float> vertices;
    const float maxRange = 230.0f;
    const float angleStep = 360.0f / rads.size();
    
    for (size_t i = 0; i < rads.size(); ++i) {
        float angle = i * angleStep * (M_PI / 180.0f);
        for (size_t j = 0; j < rads[i].size(); ++j) {
            float range = (j / static_cast<float>(rads[i].size())) * maxRange;
            float x = range * std::cos(angle);
            float y = range * std::sin(angle);
            float intensity = (rads[i][j] + 30.0f) / 90.0f; // Normalize dBZ to 0-1 range
            
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(intensity);
        }
    }
    
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(shaderProgram);
        
        // Set up transformation matrix
        glm::mat4 transform = glm::ortho(-maxRange, maxRange, -maxRange, maxRange);
        GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, vertices.size() / 3);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    
    glfwTerminate();
    return 0;
}