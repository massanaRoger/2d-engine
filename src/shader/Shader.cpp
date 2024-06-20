#include "Shader.h"
#include <fstream>
#include <iostream>
#include <sstream>

Shader::Shader(const std::string& filePath) {
    std::string fileContents;
    if (readFileToString(filePath, fileContents)) {
        std::cout << fileContents << std::endl;
    } else {
        std::cerr << "Failed to read file: " << filePath << std::endl;
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
