#include "shader.hpp"
#include "../utils/log.h"

#include <GL/glew.h>

#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

std::string Shaders::readShaderFile(const char* path) {
	std::ifstream VertexShaderStream(path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::string str;
		VertexShaderStream.seekg(0, std::ios::end);
		str.reserve(VertexShaderStream.tellg());
		VertexShaderStream.seekg(0, std::ios::beg);

		str.assign((std::istreambuf_iterator<char>(VertexShaderStream)), std::istreambuf_iterator<char>());
		VertexShaderStream.close();
		return str;
	} else {
		std::cerr << "Impossible to open file: " << path << std::endl;
		return "";
	}
}

unsigned Shaders::createAndCompileShader(std::string const &code, unsigned shaderType) {
	unsigned shaderID = glCreateShader(shaderType);
	GLint Result = GL_FALSE;
	// Compile Shader
	const char* sourcePointer = code.c_str();
	glShaderSource(shaderID, 1, &sourcePointer, NULL);
	glCompileShader(shaderID);

	// Check Shader
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &Result);
	if (Result != GL_TRUE) {
		std::cerr << "ERROR!!!";
		int infoLogLength;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0) {
			std::vector<char> shaderErrorMessage(infoLogLength + 1);
			glGetShaderInfoLog(shaderID, infoLogLength, NULL, &shaderErrorMessage[0]);
			cerr << endl << &shaderErrorMessage[0];
		} else {
			cerr << endl << "(no error description)" << endl;
		}
		std::cerr << std::endl;
		return 0;
	}
	LOGNP("OK\n");
	return shaderID;
}

unsigned Shaders::loadVertexShader(const char* path) {
	string vertexShaderCode = readShaderFile(path);
	if (vertexShaderCode == "")
		return 0;
	LOG("Compiling shader : " << path << " . . . ");
	return createAndCompileShader(vertexShaderCode, GL_VERTEX_SHADER);
}

unsigned Shaders::loadGeometryShader(const char* path) {
	string vertexShaderCode = readShaderFile(path);
	if (vertexShaderCode == "")
		return 0;
	LOG("Compiling shader : " << path << " . . . ");
	return createAndCompileShader(vertexShaderCode, GL_GEOMETRY_SHADER);
}
unsigned Shaders::loadFragmentShader(const char* path) {
	string vertexShaderCode = readShaderFile(path);
	if (vertexShaderCode == "")
		return 0;
	LOG("Compiling shader : " << path << " . . . ");
	return createAndCompileShader(vertexShaderCode, GL_FRAGMENT_SHADER);
}

unsigned Shaders::createProgram(const char* vertex_file_path, const char* fragment_file_path) {
	return createProgramGeom(vertex_file_path, nullptr, fragment_file_path);
}
unsigned Shaders::createProgramGeom(const char* vertex_file_path, const char* geom_file_path,
		const char* fragment_file_path) {
	LOGPREFIX("SHADERS");
	// Create the shaders
	unsigned vertexShaderID = loadVertexShader(vertex_file_path);
	unsigned geomShaderID = geom_file_path != nullptr ? loadGeometryShader(geom_file_path) : 0;
	unsigned fragmentShaderID = loadFragmentShader(fragment_file_path);

	if (vertexShaderID == 0 || fragmentShaderID == 0 || (geom_file_path != nullptr && geomShaderID == 0)) {
		LOGLN("Some shaders failed. Aborting...");
		return 0;
	}

	// Link the program
	LOG("Linking program . . .");
	unsigned programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, geomShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	// Check the program
	GLint Result = GL_FALSE;
	glGetProgramiv(programID, GL_LINK_STATUS, &Result);
	if (Result != GL_TRUE) {
		ERROR("Shader link ERROR!!!");
		int InfoLogLength;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			ERROR(&ProgramErrorMessage[0]);
		} else {
			ERROR("(no error description)");
		}
	} else {
		LOGNP("OK\n");
	}

	glDeleteShader(vertexShaderID);
	glDeleteShader(geomShaderID);
	glDeleteShader(fragmentShaderID);

	return programID;
}
