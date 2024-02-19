#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

struct DirectionalLight
{
    vec3 direction;
    float intensity;
};

uniform DirectionalLight light;
uniform vec3 objectColor;

void main()
{
    vec3 lightDirection = normalize(-light.direction);
    vec3 normal = normalize(Normal);
    
    float diffuseStrength = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = light.intensity * diffuseStrength * objectColor;
    
    FragColor = vec4(diffuse, 1.0);
}