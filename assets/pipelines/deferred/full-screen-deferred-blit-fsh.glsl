
void main()
{
    vec3 normal = texture(u_normalTexture, var.v_texCoord).xyz;
    vec3 light = normalize(vec3(0.1, 0.4, 0.2));
    float i = dot(normal, normal);
    if (i > 0.0) {
        i = dot(normal, light);
        color = texture(u_diffuseTexture, var.v_texCoord) * vec4(i,i,i, 1);
    }
    else {
        color = texture(u_diffuseTexture, var.v_texCoord);
    }
}
