#version 140
precision highp float;

attribute vec4 a_position; // The current position of the vertex

uniform sampler2D u_texture0; 
uniform sampler2D u_texture1; // The previous frame's output from shader 2

uniform float u_time;

uniform float u_speed_multiplier;
uniform int u_wall_strategy;
uniform int u_color_strategy;
uniform float u_random_steer_factor;
uniform float u_constant_steer_factor;
uniform float u_search_radius;
uniform float u_trail_strength;
uniform float u_vertex_radius;
uniform float u_search_angle;
uniform float u_max_distance;

// Passed to fragment shader
varying vec4 v_color;

float rand(vec2 co) {
  return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

vec3 hsv2rgb(vec3 c) {
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
  // Coord in texture space
  vec2 texcoord = vec2((a_position.x+1.0)/2.0, (a_position.y+1.0)/2.0);
  vec4 tex_val = texture2D(u_texture1, texcoord);

  // Get speed and direction
  float direction = (a_position.w-1.0)*1000.0; // Stored it in the w component
  float speed_var = (a_position.z)*1000.0; // Stored in the z component

  // Add some randomness to the direction before anything else
  direction += u_random_steer_factor*3.0*(rand(texcoord+tex_val.xy)-0.5);

  // Calculate current speed
  float speed = u_speed_multiplier * speed_var;

  // Read the underlying texture in three directions
  float sense_radius = u_search_radius;
  float sense_angle = u_search_angle;
  float sense_left = texture2D(
      u_texture1,
      vec2(
        texcoord.x+cos(direction+sense_angle)*sense_radius,
        texcoord.y+sin(direction+sense_angle)*sense_radius
        )
      ).b;
  float sense_right = texture2D(
      u_texture1,
      vec2(
        texcoord.x+cos(direction-sense_angle)*sense_radius,
        texcoord.y+sin(direction-sense_angle)*sense_radius
        )
      ).b;
  float sense_forward = texture2D(
      u_texture1,
      vec2(
        texcoord.x+cos(direction)*sense_radius,
        texcoord.y+sin(direction)*sense_radius
        )
      ).b;

  // Update direction based on sensed values
  float steer_amount = u_constant_steer_factor + u_random_steer_factor * rand(texcoord+tex_val.xy);

  // Straight ahead
  if (sense_forward > sense_left && sense_forward > sense_right) {
    direction += 0.0;
  } else if (sense_forward < sense_left && sense_forward < sense_right) { // random
    direction += u_random_steer_factor*(rand(texcoord+tex_val.xy)-0.5);
  } else if (sense_right > sense_left) {
    direction -= steer_amount; // Turn right
  } else if (sense_right < sense_left) {
    direction += steer_amount; // Turn left
  }

  // Start calculating our new position
  float y_new = a_position.y;
  float x_new = a_position.x;

  float randomAngle = 0.0;
  // Wall strategy
  switch (u_wall_strategy) {
    case 0:
      // None
      break;
    case 1:
      // wrap around
      if (y_new > 0.99) { y_new = -0.99; }
      if (y_new < -0.99) { y_new = 0.99; }

      if (x_new > 0.99) { x_new = -0.99; }
      if (x_new < -0.99) { x_new = 0.99; }
      break;
      // BounceRandom
    case 3:
      randomAngle = rand(texcoord+tex_val.xy)*u_random_steer_factor;
    case 2:
      // reverse direction if hitting wall
      if (y_new + speed*sin(direction) > 0.90) {
        float d = atan(sin(direction), cos(direction));
        direction -= 2.0*d + randomAngle;
      }
      if (y_new + speed*sin(direction) < -0.90) {
        float d = atan(sin(direction), cos(direction));
        direction -= 2.0*d + randomAngle;
      }
      if (x_new + speed*cos(direction) > 0.90) {
        float d = atan(cos(direction), sin(direction));
        direction += 2.0*d + randomAngle;
      }
      if (x_new + speed*cos(direction) < -0.90) {
        float d = atan(cos(direction), sin(direction));
        direction += 2.0*d + randomAngle;
      }
      break;
      // Slow and reverse
    case 4:
      float boundary = 0.75;
      float slowdownFactor = 0.75;

      if (y_new + speed * sin(direction) > boundary || y_new + speed * sin(direction) < -boundary) {
        speed *= slowdownFactor;
        direction = 3.14159 - direction;
      }
      if (x_new + speed * cos(direction) > boundary || x_new + speed * cos(direction) < -boundary) {
        speed *= slowdownFactor;
        direction = -direction;
      }
      break;
  }

  // Update position based on direction
  y_new += speed*u_speed_multiplier*sin(direction);
  x_new += speed*u_speed_multiplier*cos(direction);

  // Set the color of this vert
  float r = 0.0;
  float g = 0.0;
  float b = 0.0;

  // Color strategy
  switch (u_color_strategy) {
    case 0:
      r = sin(direction);
      g = cos(direction);
      b = u_trail_strength;
      break;
    case 1:
      r = speed_var*50.0;
      g = r;
      b = u_trail_strength;
      break;
    case 2:
      r = abs(y_new)/2.0 + 0.5;
      g = abs(x_new)/2.0 + 0.5;
      b = u_trail_strength;
      break;
    case 3:
      r = u_trail_strength;
      g = r;
      b = r;
      break;
      // Color strategy 4: Hue shifting based on position
    case 4:
      float distanceFromCenter = sqrt(x_new * x_new + y_new * y_new);
      float normalizedDistance = distanceFromCenter / 1.7;
      float hue = atan(y_new, x_new) / (2.0 * 3.14159) + mod(u_time/10.0,1.0);
      vec3 hsv = vec3(hue, 1.0-normalizedDistance, 1.0);
      vec3 rgb = hsv2rgb(hsv); 
      r = rgb.r;
      g = rgb.g;
      b = rgb.b;
      break;
      // Color strategy 5: Gradient based on distance from center
    case 5:
      distanceFromCenter = sqrt(x_new * x_new + y_new * y_new);
      normalizedDistance = distanceFromCenter / 1.3;
      r = mix(0.2, 1.0, normalizedDistance);
      g = mix(0.5, 1.0, normalizedDistance);
      b = u_trail_strength;
      break;

      // Color strategy 6: Color oscillation based on time
    case 6:
      float timeFactor = sin(u_time * 0.5);
      r = 0.5 + 0.5 * sin(2.0 * 3.14159 * (x_new + y_new) + timeFactor);
      g = 0.5 + 0.5 * sin(2.0 * 3.14159 * (x_new - y_new) + timeFactor);
      b = u_trail_strength;
      break;
  }

  v_color = vec4(r, g, b, 1.0);

  // Send back the position and size
  gl_Position = vec4(x_new, y_new, speed_var/1000.0, 1.0+direction/1000.0);
  gl_PointSize = u_vertex_radius;
}
