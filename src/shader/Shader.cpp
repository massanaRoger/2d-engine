#include "Shader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <glad/glad.h>

Shader::Shader(const std::string& filePath, const ShaderType shaderType): shaderID(-1) {
    std::string shaderCode;
    if (readFileToString(filePath, shaderCode)) {
        compileShader(shaderCode.c_str(), shaderType);
    } else {
        std::cerr << "Failed to read file: " << filePath << std::endl;
    }
}

void Shader::compileShader(const char* shaderCode, const ShaderType shaderType) {
    switch (shaderType) {
        case ShaderType::VertexShader:
            {
                shaderID = glCreateShader(GL_VERTEX_SHADER);
                break;
            }
        case ShaderType::FragmentShader:
            {
                shaderID = glCreateShader(GL_FRAGMENT_SHADER);
                break;
            }
        default:
            {
                shaderID = glCreateShader(GL_VERTEX_SHADER);
                break;
            }
    }

    glShaderSource(shaderID, 1, &shaderCode, nullptr);
    glCompileShader(shaderID);
    int  success;
    char infoLog[512];
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

bool Shader::readFileToString(const std::string& filePath, std::string& fileContents) {
    std::ifstream file(filePath); 

    if (!file.is_open()) {
        return false; 
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    fileContents = buffer.str();
    return true;
}

void Shader::use() {
    
}
