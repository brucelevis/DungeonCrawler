#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <stdexcept>

#include <GL/glew.h>

#include <GameLib/IO.h>
#include <GameLib/Shader.h>

namespace
{
  void _dump_source(const std::string& src)
  {
    std::string line;
    std::istringstream ss(src);
    int line_no = 1;
    while (std::getline(ss, line))
    {
      std::cout << line_no << ": " << line << std::endl;
      line_no++;
    }
  }
}

namespace gamelib
{
  Shader::Shader()
    : vertex_shader_id(0),
      fragment_shader_id(0),
      program_id(0)
  {

  }

  Shader::Shader(const std::string& vertex_shader, const std::string& fragment_shader)
  {
    loadFromFiles(vertex_shader, fragment_shader);
  }

  Shader::~Shader()
  {
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);
    glDeleteProgram(program_id);
  }

  void Shader::bindAttribute(const std::string& attrib, unsigned int location)
  {
    glBindAttribLocation(program_id, location, attrib.c_str());
  }

  void Shader::apply() const
  {
    glUseProgram(program_id);
  }

  void Shader::loadFromFiles(const std::string& vertex_shader, const std::string& fragment_shader)
  {
    std::string vertex_source = io::loadFile(vertex_shader);
    std::string fragment_source = io::loadFile(fragment_shader);
  
    loadFromMemory(vertex_source, fragment_source);

    std::cout << "Shaders " << vertex_shader << " and " << fragment_shader << " successfully loaded!" << std::endl;
  }

  void Shader::loadFromMemory(const std::string& vertex_source, const std::string& fragment_source)
  {
    vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    compileShader(vertex_source, vertex_shader_id);
    compileShader(fragment_source, fragment_shader_id);

    program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);

    int link_result = GL_FALSE;
    int info_length = 0;
    glGetProgramiv(program_id, GL_LINK_STATUS, &link_result);
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_length);

    if (link_result != GL_TRUE)
    {
      char* info_buffer = new char[info_length];
      glGetProgramInfoLog(program_id, info_length, nullptr, info_buffer);

      printf("Shader link error:\n%s\n", info_buffer);
      delete[] info_buffer;

      throw std::runtime_error("Shader link error");
    }
  }

  void Shader::compileShader(const std::string& src, unsigned int& shader_id, const std::string& file)
  {
    std::cout << "Compiling shader: " << file << std::endl;
    _dump_source(src);

    const char* ptr = src.c_str();
    glShaderSource(shader_id, 1, &ptr, nullptr);
    glCompileShader(shader_id);

    int result = GL_FALSE;
    int info_length = 0;

    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_length);

    if (result != GL_TRUE)
    {
      char* info_buffer = new char[info_length];
      glGetShaderInfoLog(shader_id, info_length, nullptr, info_buffer);

      printf("Shader \"%s\" compile error:\n%s\n", file.c_str(), info_buffer);

      delete[] info_buffer;

      throw std::runtime_error("Shader compile error!");
    }
  }
}
