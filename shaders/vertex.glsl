#version 330 core
layout(location = 0) in vec3 attr_position;
layout(location = 1) in vec3 attr_normal;
layout(location = 2) in vec2 attr_tex_coord;

out vec3 normal;
out vec2 tex_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(attr_position, 1.0);
    tex_coord = attr_tex_coord;
    normal = normalize(attr_normal);
}