#include "Shader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const std::string& vFilePath, const std::string& fFilePath) {
    std::string vShaderCode;
    if (!readFileToString(vFilePath, vShaderCode)) {
        std::cerr << "Failed to read file: " << vFilePath << std::endl;
    }

    std::string fShaderCode;
    if (!readFileToString(fFilePath, fShaderCode)) {
        std::cerr << "Failed to read file: " << vFilePath << std::endl;
    }

    compileShader(vShaderCode.c_str(), fShaderCode.c_str());
    compileProgram();
}

void Shader::compileProgram() {
    programID = glCreateProgram();

    glAttachShader(programID, m_vShaderID);
    glAttachShader(programID, m_fShaderID);
    glLinkProgram(programID);

    int  success;
    char infoLog[512];
    glGetProgramiv(programID, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(programID, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(m_vShaderID);
}

void Shader::compileShader(const char* vShaderCode, const char* fShaderCode) {
    m_vShaderID = glCreateShader(GL_VERTEX_SHADER);
    m_fShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(m_vShaderID, 1, &vShaderCode, nullptr);
    glShaderSource(m_fShaderID, 1, &fShaderCode, nullptr);
    glCompileShader(m_vShaderID);
    glCompileShader(m_fShaderID);

    int  success;
    char infoLog[512];
    glGetShaderiv(m_vShaderID, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(m_vShaderID, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    glGetShaderiv(m_fShaderID, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(m_fShaderID, 512, nullptr, infoLog);
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
    glUseProgram(programID);
}

void Shader::setVec2(const char* name, float width, float height) const {
    glUniform2f(glGetUniformLocation(programID, name), width, height);
}

void Shader::setVec3(const char* name, const glm::vec3 &vec) const {
    glUniform3f(glGetUniformLocation(programID, name), vec.x, vec.y, vec.z);
}

void Shader::setVec4(const char* name, const glm::vec4 &vec) const {
    glUniform4f(glGetUniformLocation(programID, name), vec.x, vec.y, vec.z, vec.a);
}


void Shader::setFloat(const char* name, float x) const {
    glUniform1f(glGetUniformLocation(programID, name), x);
}

void Shader::setInt(const char* name, int x) const {
    glUniform1i(glGetUniformLocation(programID, name), x);
}

void Shader::setMat4(const char* name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(programID, name), 1, GL_FALSE, glm::value_ptr(mat));
}
