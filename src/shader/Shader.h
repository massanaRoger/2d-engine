#ifndef SHADER_H
#define SHADER_H

#include <string>

class Shader {
public:
	Shader(const std::string& vFilePath, const std::string& fFilePath);
	void use();
	unsigned int programID;
private:
	bool readFileToString(const std::string& filePath, std::string& fileContents);
	void compileShader(const char* vShaderCode, const char* fShaderCode);
	void compileProgram();
	unsigned int m_vShaderID, m_fShaderID;
};

#endif
