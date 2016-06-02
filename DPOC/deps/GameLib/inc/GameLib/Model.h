#ifndef GAMELIB_MODEL_H
#define GAMELIB_MODEL_H

#include <vector>

#define VERTEX_ATTRIB_INDEX   0
#define COLOR_ATTRIB_INDEX    1
#define UV_ATTRIB_INDEX       1
#define NORMAL_ATTRIB_INDEX   2

namespace gamelib
{
  struct vertex_pack
  {
    std::vector<float> vertices;
    std::vector<float> colors;
    std::vector<float> uvs;
    std::vector<float> normals;
  };

  struct Model
  {
    Model();
    Model(const vertex_pack& _vertex_data, unsigned int _primitive_type);
    ~Model();

    void init(const vertex_pack& _vertex_data, unsigned int _primitive_type);

    void render() const;

  private:

    vertex_pack vertex_data;

    unsigned int primitive_type;

    unsigned int vertex_array_object_id[1];

    unsigned int vertex_buffer_object_id[1];
    unsigned int color_buffer_object_id[1];
    unsigned int uv_buffer_object_id[1];
    unsigned int normal_buffer_object_id[1];
  };
}

#endif
