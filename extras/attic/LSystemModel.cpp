
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT


#include "LSystemModel.h"

using namespace std;

namespace LabRender {

    /*
     V=[cc!!!&&&&&&&&&[Zp]|[Zp]]
     p=h>>>>>>>>>>>>h>>>>>>>>>>>>h
     h=[++++!F'''p]
     #-------------------------------------- Head
     H=[cccci[>>>>>dcFFF][>!!cb]]
     Code Sample 1: L-System rewriting rules for Airhorse
     */


    /*
     ---------------------------------------------------------------------------
     Turtle Orientation commands
     ---------------------------------------------------------------------------
     +     turn left around up vector
     +(x)  turn x left around up vector
     -     turn right around up vector
     -(x)  turn x right around up vector
     &     pitch down around left vector
     &(x)  pitch x down around left vector
     ^     pitch up around left vector
     ^(x)  pitch x up around left vector
     <     roll left (counter clockwise) around forward vector
     <(x)  roll x left around forward vector
     >     roll right (clockwise) around forward vector
     >(x)  roll x right around forward vector

     ---------------------------------------------------------------------------
     Special Orientation commands
     ---------------------------------------------------------------------------
     |     turn 180 deg around up vector
     %     roll 180 deg around forward vector
     $     roll until horizontal
     ~     turn/pitch/roll in a random direction
     ~(x)        "     in a random direction with a maximum of x degrees
     t     correction for gravity with 0.2
     t(x)  correction for gravity with x

     ---------------------------------------------------------------------------
     Movement commands                          when {} active
     ---------------------------------------------------------------------------
     F     move forward and draw full length    record vertex
     F(x)  move x forward and draw              record vertex
     Z     move forward and draw half length    record vertex
     Z(x)  move x forward and draw              record vertex
     f     move forward with full length        record vertex
     f(x)  move x forward                       record vertex
     z     move forward with half length        record vertex
     z(x)  move x forward                       record vertex
     g     move forward with full length        don't record vertex
     g(x)  move x forward                       don't record vertex
     .     don't move                           record vertex

     ---------------------------------------------------------------------------
     Structure commands
     ---------------------------------------------------------------------------
     [     push current state
     ]     pop current state
     {     start polygon shape
     }     end polygon shape

     ---------------------------------------------------------------------------
     Inc/Dec commands
     ---------------------------------------------------------------------------
     "     increment length with 1.1
     '     decrement length with 0.9
     "(x)  multiply length with x also '(x)
     ;     increment angle with 1.1
     :     decrement angle with 0.9
     :(x)  multiply angle with x also ;(x)
     ?     increment thickness with 1.4
     !     decrement thickness with 0.7
     ?(x)  multiply thickness with x also !(x)

     ---------------------------------------------------------------------------
     Additional commands
     ---------------------------------------------------------------------------
     c     increment color index
     c(x)  set color index to x
     @     end of object
     ---------------------------------------------------------------------------
     */

    float lscaleMax = 1;
    float lscale = 1.0f / lscaleMax;



    std::shared_ptr<Shader> lsystemShader() {
        static std::shared_ptr<Shader> shader;
        static std::once_flag init;
        std::call_once(init, [](){
            shader = std::make_shared<Shader>();
            shader->shader(std::string("LSystem vertex"),
                           Shader::ProgramType::Vertex, true, R"glsl(
                                                            uniform vec4 u_viewRect;
                                                            uniform mat4 u_view;
                                                            uniform mat4 u_viewProj;
                                                            uniform mat4 u_modelView;
                                                            uniform mat4 u_modelViewProj;
                                                            uniform mat4 u_rotationTransform;
                                                            layout(location=0) in vec3 a_position;
                                                            layout(location=1) in vec3 a_normal;
                                                            out vec4 v_pos;
                                                            out vec4 v_normal;
                                                            void main() {
                                                                vec4 pos = vec4(a_position, 1.0);
                                                                vec4 n = vec4(a_normal, 1.0);
                                                                gl_Position = u_modelViewProj * pos;
                                                                v_pos = gl_Position;
                                                                v_normal = n;
                                                            }
                                                            )glsl")
            .shader(std::string("LSystem frag"),
                    Shader::ProgramType::Fragment, true, R"glsl(
                                                       in vec2 v_normal;
                                                       in vec4 v_pos;
                                                       out vec4 color;
                                                       void main() {
                                                           color = vec4(1,0,0,1);
                                                       }
                                                       )glsl")
            .link();
        });
        return shader;
    }

    LSystemMesh::LSystemMesh() : ModelPart() {
        setVAO(std::unique_ptr<VAO>(
                new VAO(std::make_shared<Buffer<VertPN>>(
                            BufferBase::BufferType::VertexBuffer))),
               make_pair((v3f){-1.f,-1.f,0.f}, (v3f){1.f,1.f,0.f}));
        vertData = verts();
        setShader(lsystemShader());
    }
    LSystemMesh::~LSystemMesh() { }





    struct Vert { float x; float y; float z; };

    Vert vertices[] = {
        // Front Face (1-2-3-4)
        { -1.0f,  1.0f, -1.0f },
        {  1.0f,  1.0f, -1.0f },
        { -1.0f, -1.0f, -1.0f },
        {  1.0f, -1.0f, -1.0f },

        // Right Face (2-6-4-8)
        { 1.0f,  1.0f, -1.0f },
        { 1.0f,  1.0f,  1.0f },
        { 1.0f, -1.0f, -1.0f },
        { 1.0f, -1.0f,  1.0f },

        // Top Face (5-6-1-2)
        { -1.0f, 1.0f,  1.0f },
        {  1.0f, 1.0f,  1.0f },
        { -1.0f, 1.0f, -1.0f },
        {  1.0f, 1.0f, -1.0f },

        // Back Face (6-5-8-7)
        {  1.0f,  1.0f, 1.0f },
        { -1.0f,  1.0f, 1.0f },
        {  1.0f, -1.0f, 1.0f },
        { -1.0f, -1.0f, 1.0f },

        // Left Face (5-1-7-3)
        { -1.0f,  1.0f,  1.0f },
        { -1.0f,  1.0f, -1.0f },
        { -1.0f, -1.0f,  1.0f },
        { -1.0f, -1.0f, -1.0f },

        // Bottom Face (3-4-7-8)
        { -1.0f, -1.0f, -1.0f },
        {  1.0f, -1.0f, -1.0f },
        { -1.0f, -1.0f,  1.0f },
        {  1.0f, -1.0f,  1.0f }
    };

    int indices[] = {
        1,2,3,4,  // front
        6,5,8,7,  // back
        5,1,7,3,  // left
        2,6,4,8,  // right
        5,6,1,2,  // top
        3,4,7,8   // bottom
    };


    void drawUnitCube(LSystemMesh* mesh, const glm::mat4x4& transform) {
        static glm::vec3 size(1,1,1);
        static glm::vec3 c(0,0,0);
        static float sx = size.x * 0.5f;
        static float sy = size.y * 0.5f;
        static float sz = size.z * 0.5f;
        static float vertices[24*3]={
            c.x+1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz,	c.x+1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz,	c.x+1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz,	c.x+1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz,		// +X
            c.x+1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz,	c.x+1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz,	c.x+-1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz,	c.x+-1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz,		// +Y
            c.x+1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz,	c.x+-1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz,	c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz,	c.x+1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz,		// +Z
            c.x+-1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz,	c.x+-1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz,	c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz,	c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz,	// -X
            c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz,	c.x+1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz,	c.x+1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz,	c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz,	// -Y
            c.x+1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz,	c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz,	c.x+-1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz,	c.x+1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz};	// -Z

        static float normals[24*3]={
            1,0,0,	1,0,0,	1,0,0,	1,0,0,
            0,1,0,	0,1,0,	0,1,0,	0,1,0,
            0,0,1,	0,0,1,	0,0,1,	0,0,1,
            -1,0,0,	-1,0,0,	-1,0,0,	-1,0,0,
            0,-1,0,	0,-1,0,  0,-1,0,0,-1,0,
            0,0,-1,	0,0,-1,	0,0,-1,	0,0,-1};

#if 0
        static float texs[24*2]={
            0,1,	1,1,	1,0,	0,0,
            1,1,	1,0,	0,0,	0,1,
            0,1,	1,1,	1,0,	0,0,
            1,1,	1,0,	0,0,	0,1,
            1,0,	0,0,	0,1,	1,1,
            1,0,	0,0,	0,1,	1,1 };
#endif

        static uint8_t elements[6*6] ={
            0, 1, 2, 0, 2, 3,
            4, 5, 6, 4, 6, 7,
            8, 9,10, 8, 10,11,
            12,13,14,12,14,15,
            16,17,18,16,18,19,
            20,21,22,20,22,23 };

        for (int i = 0; i < 36; ++i) {
            int index = elements[i] * 3;
            glm::vec4 p(vertices[index], vertices[index+1], vertices[index+2], 1);
            glm::vec4 n(normals[index], normals[index+1], normals[index+2], 1);
            mesh->vertData->vertexData<VertPN>(true)->push_back( VertPN(glm::vec3(transform * p), glm::vec3(transform * n)) );
        }
    }



    void renderLSystem(LSystemMesh* mesh, float depth_comparison, const std::vector<LSystem::Shape>& ls, const glm::mat4x4& invCamera)
    {
        static bool init = true;
        if (init) {
            init = false;
        }

        depth_comparison = 1e6;

        float s = lscale * lscaleMax;

        // create a scale matrix
        glm::mat4x4 transform(s);

#if 0
        glBegin(GL_POINTS);
        glColor4f(0.2f,0.2f, 0.2f, 0.2f, 1);
        int count = 0;
        for (std::vector<LSystem::Shape>::const_iterator i = ls.begin(); i != ls.end(); ++i) {
            const LSystem::Shape& s = *i;
            glVertex3f(s.m[12], s.m[13], s.m[14]);
            ++count;
            if (count > foo)
                break;
        }
        glEnd();
#endif

        for (std::vector<LSystem::Shape>::const_iterator i = ls.begin(); i != ls.end(); ++i) {
            const LSystem::Shape& s = *i;
            float d = float(s.depth);
            bool draw = s.depth <= depth_comparison;
            if (!draw) {
                float d2 = depth_comparison - d;
                if (d2 > 0 && d2 < 1)
                    draw = d2 < (((rand()>>7)&0xff)/256.0f);
            }
            if (draw) {
                glm::mat4x4 t = transform * s.m;
                drawUnitCube(mesh, t);
            }
        }
    }

} // LabRender
