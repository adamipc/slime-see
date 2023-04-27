#version 120
precision highp float;

in vec4 v_color;

void main() {
  gl_FragColor = v_color;
}
