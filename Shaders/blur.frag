#version 330 core

in vec2 TexCoords;

uniform sampler2D fbt;
uniform ivec2 sResolution;

uniform int radius = 1;

out vec4 FragColor;

void main()
{
    vec3 color = vec3(0, 0, 0);
    for (int y = -1 * radius; y <= 1 * radius; ++y)
    {
        for (int x = -1 * radius; x <= 1 * radius; ++x)
        {
            color += texture(fbt, TexCoords + vec2(x / float(sResolution.x), y / float(sResolution.y))).xyz * 1.0 / pow((1 + 2 * radius), 2);
        }
    }

    FragColor = vec4(color, 1.0);
}
