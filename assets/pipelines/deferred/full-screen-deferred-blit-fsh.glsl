
void main() 
{
    vec3 normal = texture(u_normalTexture, vert.v_texCoord).xyz;
    vec3 light = normalize(vec3(0.1, 0.4, 0.2));
    float i = dot(normal, normal);
    if (i > 0.0) {
        i = dot(normal, light);
        o_colorTexture = texture(u_diffuseTexture, vert.v_texCoord) * vec4(i,i,i, 1);
    }
    else {
        o_colorTexture = texture(u_diffuseTexture, vert.v_texCoord);
    }
}
