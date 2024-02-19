#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 lightDirection;
uniform float lightCutoffAngle;
uniform vec3 objectColor;

void main()
{
    // Calculate the light direction and distance
    vec3 lightDir = normalize(lightPosition - FragPos);
    float distance = length(lightPosition - FragPos);

    // Calculate the spotlight cone direction and intensity
    vec3 coneDir = normalize(lightDirection);
    float coneAngle = degrees(acos(dot(-lightDir, coneDir)));
    float coneIntensity = smoothstep(lightCutoffAngle - 5.0, lightCutoffAngle, coneAngle);

    // Calculate diffuse and specular lighting
    vec3 normal = normalize(Normal);
    vec3 diffuse = max(dot(normal, -lightDir), 0.0) * lightColor;
    vec3 specular = vec3(0.0);

    if (dot(normal, -lightDir) > 0.0) {
        vec3 viewDir = normalize(-FragPos);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float specularIntensity = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
        specular = specularIntensity * lightColor;
    }

    // Combine diffuse and specular lighting
    vec3 lighting = (diffuse + specular) * coneIntensity;

    // Calculate final fragment color
    FragColor = vec4(objectColor * lighting, 1.0);
}