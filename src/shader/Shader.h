#ifndef SHADER_H
#define SHADER_H

#include <string>
class Shader {
public:
	Shader(const std::string& filePath);

private:
	bool readFileToString(const std::string& filePath, std::string& fileContents);
};

#endif
