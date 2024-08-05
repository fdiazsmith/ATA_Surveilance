#ifndef OPENGL_UTILS_H
#define OPENGL_UTILS_H

#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>
#include <string>

GLuint compile_shader(GLenum type, const std::string& source);
GLuint create_shader_program(const std::string& vertex_shader_path, const std::string& fragment_shader_path);
GLuint create_texture(GLsizei width, GLsizei height);
void update_texture(GLuint texture, const void* data, GLsizei width, GLsizei height);
void set_uniform_float(GLuint program, const std::string& name, float value); // Add this line


#endif // OPENGL_UTILS_H
