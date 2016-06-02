#ifndef GAMELIB_SHADER_H
#define GAMELIB_SHADER_H

#include <string>
#include <map>

namespace gamelib
{
  struct Shader
  {
    Shader();
    Shader(const std::string& vertex_shader, const std::string& fragment_shader);
    ~Shader();

    void loadFromFiles(const std::string& vertex_shader, const std::string& fragment_shader);
    void loadFromMemory(const std::string& vertex_source, const std::string& fragment_source);

    void bindAttribute(const std::string& attrib, unsigned int location);
    operator unsigned int() const { return program_id;  }

    void apply() const;
  private:
    void compileShader(const std::string& src, unsigned int& shader_id, const std::string& file = "");
  private:
    unsigned int vertex_shader_id;
    unsigned int fragment_shader_id;
    unsigned int program_id;
  };
}

#endif
