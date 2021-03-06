#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 frag_uv;

layout (location = 0) out vec4 frag_color;

void main() 
{
  	frag_color = vec4(frag_uv, 0.0f, 1.0f);
}