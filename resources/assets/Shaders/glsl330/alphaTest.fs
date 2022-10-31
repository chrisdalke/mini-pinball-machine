#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord)*colDiffuse*fragColor;
    if (texelColor.r < 0.01)
        discard;
    if (texelColor.r < 0.6){
        texelColor.a = 0.7;
    } else {
        texelColor.a = 0.7;
    }
    finalColor = vec4(0.00784313725, 0.48627451, 0.933333333, texelColor.a);
    //if (texelColor.r < 0.1)
    //    finalColor = vec4(1.0,1.0,1.0, 1.0);
}
