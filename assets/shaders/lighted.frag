#version 330

#define MAX_LIGHTS 8

// Light types
#define DIRECTIONAL 0
#define POINT 1
#define SPOT 2

// Struct to represent a light
struct Light {
    int type;
    vec3 color;
    vec3 position;
    vec3 direction;
    vec3 attenuation;
    vec2 cone_angles;
};

// Struct to represent the sky
struct Sky {
    vec3 top;
    vec3 middle;
    vec3 bottom;
};

uniform Light lights[MAX_LIGHTS];
uniform int light_count;
uniform Sky sky;

// Struct to represent the material
struct Material {
    sampler2D albedo;
    sampler2D emissive;
    sampler2D specular;
    sampler2D roughness;
    sampler2D ambient_occlusion;
};

uniform Material material;

// Varying inputs from the vertex shader
in Varyings {
    vec4 color;
    vec2 tex_coord;
    vec3 normal;
    vec3 view;
    vec3 world;
} fs_in;

// Output color of the fragment
out vec4 frag_color;

void main() {
    // Normalize vectors
    vec3 view = normalize(fs_in.view);
    vec3 normal = normalize(fs_in.normal);

    // Retrieve material properties
    vec3 material_diffuse = texture(material.albedo, fs_in.tex_coord).rgb;
    vec3 material_specular = texture(material.specular, fs_in.tex_coord).rgb;
    float material_roughness = texture(material.roughness, fs_in.tex_coord).r;
    float material_shininess = 2.0 / pow(clamp(material_roughness, 0.001, 0.999), 4.0) - 2.0;
    vec3 material_ambient = material_diffuse * texture(material.ambient_occlusion, fs_in.tex_coord).r;
    vec3 material_emissive = texture(material.emissive, fs_in.tex_coord).rgb;

    // Compute sky light
    vec3 sky_light = mix(sky.middle, (normal.y > 0) ? sky.top : sky.bottom, normal.y * normal.y);

    // Initialize fragment color with emissive and ambient components
    frag_color = vec4(material_emissive + material_ambient, 1.0);

    // Clamp the number of lights to the maximum allowed
    int clamped_light_count = min(MAX_LIGHTS, light_count);

    // Compute lighting contributions
    for (int i = 0; i < clamped_light_count; ++i) {
        Light light = lights[i];
        vec3 direction_to_light = (light.type == DIRECTIONAL) ? -light.direction : normalize(light.position - fs_in.world);
        
        vec3 diffuse = light.color * material_diffuse * max(0, dot(normal, direction_to_light));
        vec3 reflected = reflect(-direction_to_light, normal);
        vec3 specular = light.color * material_specular * pow(max(0, dot(view, reflected)), material_shininess);

        float attenuation = 1.0;
        if (light.type != DIRECTIONAL) {
            if (light.type == POINT) {
                float distance_to_light = distance(light.position, fs_in.world);
                attenuation = 1.0 / (1.0 + light.attenuation.x * distance_to_light + light.attenuation.y * distance_to_light * distance_to_light);
            }
            
            if (light.type == SPOT) {
                float angle = acos(dot(-direction_to_light, light.direction));
                attenuation *= smoothstep(light.cone_angles.y, light.cone_angles.x, angle);
            }
        }

        frag_color.rgb += (diffuse + specular) * attenuation;
    }
}
