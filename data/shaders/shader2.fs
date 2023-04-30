#version 140
precision highp float;
uniform sampler2D u_texture0; // A texture input - the output of shader 1
uniform sampler2D u_texture1; // A texture input - the previous frame's output from shader 2
uniform float u_fade_speed; // TODO
uniform float u_blur_fraction; // TODO

uniform float u_time;

varying vec4 loc; // from the vertex shader, used to compute texture locations

// For blurring
const float Directions = 8.0;
const float Quality = 1.0; // 3 for snowflake
const float Radius = 1.0/1800.0; // TODO pass in resolution
float pixelCount = 1.0;

void main() {

  // Convert the clip-space coordinates into texture space ones
  vec2 texcoord = vec2((loc.x+1.0)/2.0, (loc.y+1.0)/2.0); 

  // Gaussian Blur 
  vec4 blurred = texture2D(u_texture1, texcoord); // sample the previous frame    
  for( float d=0.0; d<6.3; d+=6.3/Directions){
    for(float i=1.0/Quality; i<=1.0; i+=1.0/Quality){
      blurred += texture2D(u_texture1, texcoord+vec2(cos(d),sin(d))*Radius*i); 		
      pixelCount += 1.0;
    }
  }
  blurred /= pixelCount;      

  vec4 shader1_out = texture2D(u_texture0, texcoord); // The output of shader 1
  vec4 prev_frame = texture2D(u_texture1, texcoord); // The output of shader 2 (previous frame)

  // Modify how much blurring by mixing the blurred version with the original
  blurred = prev_frame*(1.0-u_blur_fraction) + blurred*u_blur_fraction;

  // The output colour - adding the shader 1 output to the blurred version of the previous frame
  gl_FragColor = shader1_out + blurred*(1.0-u_fade_speed) - 0.0001;
}
