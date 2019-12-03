#version 450

in vec4 position;
in vec4 color;
in vec3 normal;
in vec2 uv_in;

out vec4 color_out;
out vec2 uv;

uniform float scale;
uniform mat4 Mat;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * Mat * position;
	//uv = uv_in;
	color_out =  color;
}