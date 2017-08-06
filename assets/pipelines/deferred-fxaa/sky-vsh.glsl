
// sky vsh
void main() {
	vec4 pos = vec4(a_position, 1.0);
	vert.v_eyeDirection = (u_skyMatrix * pos).xyz;
	pos.z = 1.0; // maximum depth value as sentinel to enable writing
	gl_Position = pos;
}
