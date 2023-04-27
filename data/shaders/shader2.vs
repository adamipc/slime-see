#version 120
attribute vec2 a_vertex;

varying vec4 loc; // location in clip space
uniform float u_time;

void main(void) {
  gl_Position = vec4(a_vertex.x, a_vertex.y, 0.0, 1.0);
  loc = gl_Position; // pass to frag shader
}
