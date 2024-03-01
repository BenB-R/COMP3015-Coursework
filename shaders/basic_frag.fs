#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;

uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform sampler2D texture_diffuse1;

// Predefined fog values
vec3 fogColor = vec3(0.1, 0.1, 0.3);
float fogStart = 30.0; // Fog starts at a distance of 20 units from the view
float fogEnd = 150.0; // Fog completely obscures objects beyond 100 units


void main() {
    // Light properties
    vec3 lightColor = vec3(2.5, 1.2, 0.2); // Much brighter, red-orange for fire

    vec3 ambientColor = vec3(0.3, 0.3, 0.3); // Slightly purple-blue for general lighting
    vec3 specularColor = vec3(1.0, 0.8, 0.6); // Subdued specular highlights, adjust if needed

    // Attenuation coefficients (adjusted for further reach)
    float constant = 1.0;
    float linear = 0.07; // Slightly lower to allow light to travel further
    float quadratic = 0.017; // Slightly lower to allow light to travel further

    // Material properties
    float shininess = 32.0;

    // Ambient
    vec3 ambient = ambientColor;

    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);  
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * diff; 

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularColor * spec;

    // Attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));
    diffuse *= attenuation;
    specular *= attenuation;

    vec3 objectColor = texture(texture_diffuse1, TexCoords).rgb;
    vec3 result = (ambient + diffuse + specular) * objectColor;

    // Fog calculation
    float fogFactor = clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);
    vec3 finalColor = mix(fogColor, result, fogFactor); // Linear blend between fog color and fragment color

    FragColor = vec4(finalColor, 1.0);
}
