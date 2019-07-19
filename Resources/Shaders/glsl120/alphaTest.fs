#version 120

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add here your custom variables

void main()
{
    vec4 texelColor = texture2D(texture0, fragTexCoord)*colDiffuse*fragColor;
    if (texelColor.r < 0.01)
        discard;
    if (texelColor.r < 0.5){
        texelColor.a = 0.6;
    } else {
        texelColor.a = 0.9;
    }
    gl_FragColor = vec4(0.00784313725, 0.48627451, 0.933333333, texelColor.a);
}
