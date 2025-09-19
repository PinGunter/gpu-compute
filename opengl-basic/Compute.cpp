#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

// ¡Importante! Incluir GLAD antes que GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// El resto del código (constantes y funciones de ayuda) es idéntico al anterior...
// --- Constantes ---
const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;

// --- Funciones de ayuda ---
std::string loadShaderSource(const char* filePath);
GLuint compileShader(GLenum type, const char* source);
GLuint createShaderProgram(const char* vsSource, const char* fsSource);
GLuint createComputeShaderProgram(const char* csSource);


int main() {
    // --- 1. Inicialización de GLFW (sin cambios) ---
    if (!glfwInit()) {
        std::cerr << "Fallo al inicializar GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Compute Shader MVP (GLAD)", NULL, NULL);
    if (!window) {
        std::cerr << "Fallo al crear la ventana GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // --- 2. Inicialización de GLAD (¡Este es el cambio!) ---
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Fallo al inicializar GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // --- El resto del programa (del paso 3 al 7) es exactamente el mismo ---
    // Cargar shaders, crear textura, configurar quad, bucle principal y limpieza
    // no necesitan ninguna modificación. Pega aquí el resto del código
    // del ejemplo anterior.

    // ... (Cargar shaders, crear textura, quad, etc.)
    // --- 3. Cargar y compilar shaders ---
    std::string computeShaderSource = loadShaderSource("Shaders/compute.glsl");
    std::string vertexShaderSource = loadShaderSource("Shaders/vertex.glsl");
    std::string fragmentShaderSource = loadShaderSource("Shaders/fragment.glsl");

    GLuint computeProgram = createComputeShaderProgram(computeShaderSource.c_str());
    GLuint quadProgram = createShaderProgram(vertexShaderSource.c_str(), fragmentShaderSource.c_str());

    // --- 4. Crear la textura de destino ---
    GLuint textureID;
    glGenTextures(1, &textureID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindImageTexture(0, textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // --- 5. Configurar el quad que cubrirá toda la pantalla ---
    float quadVertices[] = {
        -1.0f,  1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f
    };
    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // --- 6. Bucle principal de renderizado ---
    while (!glfwWindowShouldClose(window)) {
        glUseProgram(computeProgram);
        glUniform1f(glGetUniformLocation(computeProgram, "time"), (float)glfwGetTime());
        glDispatchCompute((GLuint)WINDOW_WIDTH / 16, (GLuint)WINDOW_HEIGHT / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(quadProgram);
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- 7. Limpieza ---
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteProgram(computeProgram);
    glDeleteProgram(quadProgram);
    glDeleteTextures(1, &textureID);

    glfwTerminate();
    return 0;
}



std::string loadShaderSource(const char* filePath) {
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo de shader: " << filePath << std::endl;
        return "";
    }
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    return shaderStream.str();
}

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Error de compilación de shader: " << infoLog << std::endl;
    }
    return shader;
}

GLuint createShaderProgram(const char* vsSource, const char* fsSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Error de enlazado de programa: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

GLuint createComputeShaderProgram(const char* csSource) {
    GLuint computeShader = compileShader(GL_COMPUTE_SHADER, csSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, computeShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Error de enlazado de programa de computación: " << infoLog << std::endl;
    }

    glDeleteShader(computeShader);
    return program;
}