#version 330

uniform vec3 eye;
uniform mat4 VP;
uniform mat4 M;
uniform mat4 M_IT;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;
layout(location = 3) in vec3 normal;

out Varyings {
    vec4 color;
    vec2 tex_coord;
    vec3 normal;
    vec3 view;
    vec3 world;
} vs_out;

void main() {
    // Transform position to world space
    vec4 world_position = M * vec4(position, 1.0);
    vec3 world = world_position.xyz;
    
    // Transform world position to clip space
    gl_Position = VP * vec4(world, 1.0);
    
    // Pass attributes to fragment shader
    vs_out.color = color;
    vs_out.tex_coord = tex_coord;
    
    // Transform normal to view space
    vs_out.normal = normalize((M_IT * vec4(normal, 0.0)).xyz);
    
    // Calculate view vector in view space
    vs_out.view = eye - world;
    
    // Pass world position to fragment shader
    vs_out.world = world;
}
