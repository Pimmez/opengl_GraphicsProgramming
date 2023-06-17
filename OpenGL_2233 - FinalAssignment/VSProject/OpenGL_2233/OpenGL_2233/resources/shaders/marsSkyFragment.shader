#version 330 core
out vec4 FragColor;

in vec4 worldPosition;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;

vec3 lerp(vec3 a, vec3 b, float t)
{
    return a + (b - a) * t;
}

void main()
{
    vec3 topColor = vec3(255.0/255.0, 100.0/255.0, 130.0/255.0);
    vec3 botColor = vec3(200.0/255.0, 100.0/255.0, 150.0/255.0);
    
    vec3 sunColor = vec3(1.0, 50.0 / 255.0, 50.0 / 255.0);
    
    //Calculate View
    vec3 viewDir = normalize(worldPosition.rgb - cameraPosition); 
    float sun = max(pow(dot(-viewDir, lightDirection), 128), 0.0);
    
    FragColor = vec4(lerp(botColor, topColor, max(viewDir.y, 0.0)) + sun * sunColor, 1);
}