void main() {
	vert.v_texCoord = a_uv;
	gl_Position = vec4(a_position, 1.0);
}
