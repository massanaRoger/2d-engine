#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glm/fwd.hpp>

class Shader {
public:
	Shader(const std::string& vFilePath, const std::string& fFilePath);
	void setVec2(const char* name, float width, float height) const;
	void setVec3(const char* name, const glm::vec3 &vec) const;
	void setMat4(const char* name, const glm::mat4 &mat) const;
	void setFloat(const char* name, float x) const;
	void setInt(const char* name, int x) const;
	void use();
	unsigned int programID;
private:
	bool readFileToString(const std::string& filePath, std::string& fileContents);
	void compileShader(const char* vShaderCode, const char* fShaderCode);
	void compileProgram();
	unsigned int m_vShaderID, m_fShaderID;
};

#endif
