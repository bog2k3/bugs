#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>

class Shaders {
public:
	static GLuint loadVertexShader(const char* path);
	static GLuint loadGeometryShader(const char* path);
	static GLuint loadFragmentShader(const char* path);
	static GLuint createProgram(const char* vertex_file_path, const char* fragment_file_path);
	static GLuint createProgramGeom(const char* vertex_file_path, const char* geom_file_path,
			const char* fragment_file_path);

private:
	Shaders() {
	}
	static std::string readShaderFile(const char* path);
	static GLuint createAndCompileShader(std::string const &code, GLuint shaderType);
};

#endif
