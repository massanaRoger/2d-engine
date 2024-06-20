#ifndef SHADER_H
#define SHADER_H

#include <string>

enum class ShaderType {
	VertexShader,
	FragmentShader
};

class Shader {
public:
	Shader(const std::string& filePath, const ShaderType shaderType);
	void use();
	unsigned int shaderID;
private:
	bool readFileToString(const std::string& filePath, std::string& fileContents);
	void compileShader(const char* shaderCode, const ShaderType shaderType); 
};

#endif
