#version 330 core

out vec4 frag_color;

// In this shader, we want to draw a checkboard where the size of each tile is (size x size).
// The color of the top-left most tile should be "colors[0]" and the 2 tiles adjacent to it
// should have the color "colors[1]".

//TODO (DONE): (Req 1) Finish this shader.

uniform int size = 32;
uniform vec3 colors[2];

void main(){

    int x = int(gl_FragCoord.x);
    int y = int(gl_FragCoord.y);

    bool tileX = (x % (2 * size) < size);
    bool tileY = (y % (2 * size) < size);

    tileX == tileY ?  frag_color = vec4(colors[0], 1.0) : frag_color = vec4(colors[1], 1.0);
}