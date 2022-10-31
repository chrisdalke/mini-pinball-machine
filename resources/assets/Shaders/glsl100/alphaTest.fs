#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add here your custom variables

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture2D(texture0, fragTexCoord)*colDiffuse*fragColor;
    if (texelColor.r < 0.01)
        discard;
    gl_FragColor = vec4(0.00784313725, 0.48627451, 0.933333333, 0.8);
}
