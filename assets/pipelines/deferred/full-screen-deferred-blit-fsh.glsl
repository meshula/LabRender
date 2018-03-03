
out vec4 fragColor;

void main() {
    vec3 normal = texture(u_normalTexture, vert.v_texCoord).xyz;
    vec3 light = normalize(vec3(0.1, 0.4, 0.2));
    float i = dot(normal, normal);
    if (i == 1.0) {
        discard;
    }
    else {
        i = dot(normal, light);
        fragColor = vec4(i,i,i, 1);
    }
}
