#ifndef DPOC_STAGING_WATEREFFECT_H_
#define DPOC_STAGING_WATEREFFECT_H_

const char* const ripple =
"uniform float time;\n"
"uniform sampler2D texture;\n"
"void main() {\n"
" float factor = 1.f;//(2*3.141592f) / 0.5f;\n"
" vec2 texCoord = gl_TexCoord[0].xy;\n"
" texCoord.x += sin(texCoord.y+time*factor)*0.05f;\n"
" vec4 color = texture2D(texture, texCoord);\n"
" gl_FragColor = vec4(color.r/4, color.g/2, color.b, color.a);  //texture2D(texture, texCoord);\n"
"}";

//static bool hasShader = false;
//static sf::Shader shader;
//static float time = 0.f;
//if (!hasShader)
//{
//  hasShader = true;
//  shader.loadFromMemory(ripple, sf::Shader::Fragment);
//}
//time += 0.01f;

//shader.setParameter("texture", sf::Shader::CurrentTexture);
//shader.setParameter("time", time);
//m_targetTexture.draw(sprite, &shader);

#endif /* DPOC_STAGING_WATEREFFECT_H_ */
