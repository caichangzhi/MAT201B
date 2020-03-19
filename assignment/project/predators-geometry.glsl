#version 400

layout(points) in;
layout(triangle_strip, max_vertices = 9) out;

in Vertex1 {
  vec3 position;
  vec3 forward;
  vec3 up;
  //vec3 color;
}
vertex1[];

out Fragment1 {  //
  vec4 color;
}
fragment1;

uniform mat4 al_ProjectionMatrix;
uniform mat4 al_ModelViewMatrix;

uniform float size;
uniform float ratio;

mat4 rotationMatrix(vec3 axis, float angle) {
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;

  return mat4(
      oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s,
      oc * axis.z * axis.x + axis.y * s, 0.0, oc * axis.x * axis.y + axis.z * s,
      oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0,
      oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s,
      oc * axis.z * axis.z + c, 0.0, 0.0, 0.0, 0.0, 1.0);
}

void main() {
  mat4 pm = al_ProjectionMatrix * al_ModelViewMatrix;
  vec3 position = vertex1[0].position;
  vec3 forward = vertex1[0].forward * size;
  vec3 up = vertex1[0].up;

  vec3 over = cross(up, forward) * ratio;

  vec4 a = pm * vec4(position + forward, 1.0);
  vec4 b = pm * vec4(position + (rotationMatrix(forward, radians(60)) * vec4(over, 0.0)).xyz, 1.0);
  vec4 c = pm * vec4(position + (rotationMatrix(forward, radians(180)) * vec4(over, 0.0)).xyz, 1.0);
  vec4 d = pm * vec4(position + (rotationMatrix(forward, radians(300)) * vec4(over, 0.0)).xyz, 1.0);

  gl_Position = a;
  fragment1.color = vec4(1.0, 1.0, 1.0, 1.0);
  EmitVertex();

  gl_Position = b;
  fragment1.color = vec4(1.0, 1.0, 1.0, 1.0);
  EmitVertex();

  gl_Position = c;
  fragment1.color = vec4(1.0, 1.0, 1.0, 1.0);
  EmitVertex();

  EndPrimitive();

  gl_Position = a;
  fragment1.color = vec4(1.0, 1.0, 1.0, 1.0);
  EmitVertex();

  gl_Position = c;
  fragment1.color = vec4(1.0, 1.0, 1.0, 1.0);
  EmitVertex();

  gl_Position = d;
  fragment1.color = vec4(1.0, 1.0, 1.0, 1.0);
  EmitVertex();

  EndPrimitive();

  gl_Position = a;
  fragment1.color = vec4(1.0, 1.0, 1.0, 1.0);
  EmitVertex();

  gl_Position = d;
  fragment1.color = vec4(1.0, 1.0, 1.0, 1.0);
  EmitVertex();

  gl_Position = b;
  fragment1.color = vec4(1.0, 1.0, 1.0, 1.0);
  EmitVertex();

  EndPrimitive();
}
