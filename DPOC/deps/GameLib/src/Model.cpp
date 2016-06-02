#include <cassert>
#include <cstdio>

#include <GL/glew.h>

#include <GameLib/Model.h>

namespace gamelib
{
  Model::Model()
    : primitive_type(0)
  {
  }

  Model::Model(const vertex_pack& _vertex_data, unsigned int _primitive_type)
    : primitive_type(0)
  {
    init(_vertex_data, _primitive_type);
  }

  Model::~Model()
  {
  }

  void Model::init(const vertex_pack& _vertex_data, unsigned int _primitive_type)
  {
    vertex_data = _vertex_data;
    primitive_type = _primitive_type;

    assert(vertex_data.vertices.size());

    glGenVertexArrays(1, &vertex_array_object_id[0]);
    glBindVertexArray(vertex_array_object_id[0]);

    glGenBuffers(1, vertex_buffer_object_id);
    glEnableVertexAttribArray(VERTEX_ATTRIB_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_id[0]);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.vertices.size() * sizeof(GLfloat), &vertex_data.vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(VERTEX_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    if (vertex_data.colors.size())
    {
      glGenBuffers(1, color_buffer_object_id);
      glEnableVertexAttribArray(COLOR_ATTRIB_INDEX);
      glBindBuffer(GL_ARRAY_BUFFER, color_buffer_object_id[0]);
      glBufferData(GL_ARRAY_BUFFER, vertex_data.colors.size() * sizeof(GLfloat), &vertex_data.colors[0], GL_STATIC_DRAW);
      glVertexAttribPointer(COLOR_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    if (vertex_data.uvs.size())
    {
      glGenBuffers(1, uv_buffer_object_id);
      glEnableVertexAttribArray(UV_ATTRIB_INDEX);
      glBindBuffer(GL_ARRAY_BUFFER, uv_buffer_object_id[0]);
      glBufferData(GL_ARRAY_BUFFER, vertex_data.uvs.size() * sizeof(GLfloat), &vertex_data.uvs[0], GL_STATIC_DRAW);
      glVertexAttribPointer(UV_ATTRIB_INDEX, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    if (vertex_data.normals.size())
    {
      glGenBuffers(1, normal_buffer_object_id);
      glEnableVertexAttribArray(NORMAL_ATTRIB_INDEX);
      glBindBuffer(GL_ARRAY_BUFFER, normal_buffer_object_id[0]);
      glBufferData(GL_ARRAY_BUFFER, vertex_data.normals.size() * sizeof(GLfloat), &vertex_data.normals[0], GL_STATIC_DRAW);
      glVertexAttribPointer(NORMAL_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    glBindVertexArray(0);
  }

  void Model::render() const
  {
    glBindVertexArray(vertex_array_object_id[0]);
    glDrawArrays(primitive_type, 0, vertex_data.vertices.size() / 3);

    // Unbind vertex array object.
    glBindVertexArray(0);
  }
}
