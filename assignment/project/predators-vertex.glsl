#version 400

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color; // a not used
layout(location = 2) in vec2 tex; // s,t not used
layout(location = 3) in vec3 normal;

out Vertex1 {
  vec3 position;
  vec3 forward;
  vec3 up;
  //vec3 color;
}
vertex1;

void main() {
  vertex1.position = position;
  vertex1.forward = normal;
  vertex1.up = color.rgb;
  // vertex.color = vec3(color.a, tex.s, tex.t);
}
