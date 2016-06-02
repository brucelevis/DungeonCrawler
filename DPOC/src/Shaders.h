#ifndef SHADERS_H_
#define SHADERS_H_

const char* const Shader_LightShader_Vertex =
"#version 330\n"

"layout(location = 0) in vec3 vertex_position;\n"
"layout(location = 1) in vec2 vertex_uv;\n"
"layout(location = 2) in vec3 vertex_normal;\n"

"uniform mat4 MVP;\n"
"uniform mat4 ModelMatrix;\n"
"uniform mat4 ViewMatrix;\n"
"uniform mat4 InverseTransposedModelMatrix;\n"

"// Light parameters\n"
"uniform vec3 LightPosition;\n"

"// Texture and colour components.\n"
"out vec2 uv;\n"

"// Positions and directions.\n"
"out vec3 Position_WorldSpace;\n"
"out vec3 Normal_CameraSpace;\n"
"out vec3 EyeDirection_CameraSpace;\n"
"out vec3 LightDirection_CameraSpace;\n"

"void main()\n"
"{\n"
"  gl_Position = MVP * vec4(vertex_position, 1);\n"

"  // OUT\n"
"  Position_WorldSpace = (ModelMatrix * vec4(vertex_position, 1)).xyz;\n"

"  // OUT\n"
"  vec3 VertexCameraSpace = (ViewMatrix * ModelMatrix * vec4(vertex_position, 1)).xyz;\n"
"  EyeDirection_CameraSpace = vec3(0, 0, 0) - VertexCameraSpace;\n"

"  // OUT\n"
"  vec3 LightPosition_CameraSpace = (ViewMatrix * vec4(LightPosition, 1)).xyz;\n"
"  LightDirection_CameraSpace = LightPosition_CameraSpace + EyeDirection_CameraSpace;\n"

"  // OUT\n"
"  Normal_CameraSpace = (InverseTransposedModelMatrix * vec4(vertex_normal, 0)).xyz;\n"

"  // Rest of this stuff.\n"
"  uv = vertex_uv;\n"
"}\n";

const char* const Shader_LightShader_Fragment =
"#version 330\n"

"in vec2 uv;\n"

"in vec3 Position_WorldSpace;\n"
"in vec3 Normal_CameraSpace;\n"
"in vec3 EyeDirection_CameraSpace;\n"
"in vec3 LightDirection_CameraSpace;\n"

"out vec3 color;\n"

"uniform vec3 LightPosition;\n"
"uniform vec3 Ka;\n"
"uniform vec3 Kd;\n"
"uniform vec3 Ks;\n"
"uniform sampler2D texture0;\n"

"void main(void)\n"
"{\n"
"  vec2 inverted = vec2(uv.x, 1.0f - uv.y);\n"

"  // Constants\n"
"  const vec3 LightColor = vec3(0.3f, 0.3f, 0.3f);\n"
"  const float LightPower = 10.0f;\n"
"  const float shininess = 5;\n"

"  vec3 DiffuseColor = texture(texture0, inverted).rgb;\n"
"  vec3 AmbientColor = Ka * DiffuseColor;\n"
"  vec3 SpecularColor = Ks;\n"

"  float distance = length( LightPosition - Position_WorldSpace );\n"

"  vec3 Normal = normalize( Normal_CameraSpace );\n"
"  vec3 Direction = normalize( LightDirection_CameraSpace );\n"
"  float cosTheta = clamp( abs(dot( Normal, Direction)), 0, 1 );\n"

"  vec3 Eye = normalize( EyeDirection_CameraSpace );\n"
"  vec3 Reflect = reflect(-Direction, Normal);\n"
"  float cosAlpha = clamp( dot(Eye, Reflect), 0, 1 );\n"

"  vec3 lighting_color = AmbientColor + \n"
"    DiffuseColor * LightColor * LightPower * cosTheta / (distance * distance) +\n"
"  SpecularColor * LightColor * LightPower * pow(cosAlpha, shininess) / (distance * distance);\n"
"  vec3 texture_color = texture(texture0, inverted).rgb;\n"

"  color = vec3( min(lighting_color.r, texture_color.r),\n"
"                min(lighting_color.g, texture_color.g),\n"
"                min(lighting_color.b, texture_color.b) );\n"
"  //color = texture(texture0, inverted).rgb;\n"
"}\n";

#endif /* SHADERS_H_ */
