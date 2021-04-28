#version 430 core

in vec3 normal;
in vec2 tex_coord;

out vec4 frag_color;

layout(location = 0) uniform vec3 color_diffuse;
layout(location = 1) uniform vec3 color_ambient;
layout(location = 2) uniform vec3 color_specular;
layout(location = 3) uniform vec3 color_emissive;

layout(location = 4) uniform float material_transparency;
layout(location = 5) uniform float material_refraction;
layout(location = 6) uniform float material_specular_exp;

uniform sampler2D texture_diffuse;

void main() {

    float global_ambient_intensity = 0.5f;
    vec3 global_ambient_color = global_ambient_intensity * vec3(0.86, 0.94, 1);
    vec3 ambient = global_ambient_color * color_ambient;

    float global_sun_intensity = 1.3f;
    vec3 global_sun_color = global_sun_intensity * vec3(1, 0.98, 0.95);
    vec3 global_sun_dir = normalize(vec3(1.0f, -1.0f, 1.0f));
    float sun_angle = max(dot(normal, global_sun_dir), 0.0f);
    vec3 diffuse = sun_angle * global_sun_color * color_diffuse;

    vec3 flat_color = diffuse + ambient;

    vec4 tex_color = texture(texture_diffuse, tex_coord);
    if(tex_color.a == 0.0f)
        discard;

    frag_color = vec4(flat_color, 1 - material_transparency) * tex_color;
}