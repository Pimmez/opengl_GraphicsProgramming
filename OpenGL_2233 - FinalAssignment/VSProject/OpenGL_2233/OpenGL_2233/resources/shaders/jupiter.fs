#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normals;
in vec4 FragPos;

uniform sampler2D diffuse;

uniform vec3 cameraPosition;
uniform vec3 lightDirection;

vec4 lerp(vec4 a, vec4 b, float t) {
    return a + (b - a) * t;
}

vec3 lerp(vec3 a, vec3 b, float t) {
    return a + (b - a) * t;
}

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void main()
{    
    vec4 diffuseColor = texture(diffuse, TexCoords);

    float light = max(dot(-lightDirection, Normals + 0.25), 0.0); //set de edge iets meer naar achter met +.25
    light = pow(light * 32.0, 2.0) / 32.0; //16 is de edge waarden van de light/dark planet edge
    light = max(min(light, 1.0), 0.0);

    vec3 viewDir = normalize(FragPos.rgb - cameraPosition);
    vec3 refl = reflect(lightDirection, Normals);
    float spec = pow(max(dot(-viewDir, refl), 0.0), 6.0);
    
    float fresnel = pow(max(1.0 - dot(-viewDir, Normals), 0.0), 3.0);

    vec3 specular = spec  + fresnel * 2* vec3(0.1, 0.3 ,0.1);

    vec4 output = diffuseColor * light + vec4(specular, 0);

    FragColor = output;
}