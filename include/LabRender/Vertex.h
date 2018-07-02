//
//  Vertex.h
//  LabApp
//
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#pragma once

#include <LabRender/LabRender.h>
#include <LabRender/ErrorPolicy.h>
#include <LabRender/MathTypes.h>
#include <LabRender/Semantic.h>
#include <LabRender/SemanticType.h>

#include <memory>
#include <ostream>
#include <iostream>

// OSX defines check as a macro <sob>
#ifdef check
#   undef check
#endif

namespace lab {
    class VAO;

    // BufferBase provides a vertex layout of attribute names, semantics, and a stride
    // The templated subclasses provide the actual vertex data
    // functionality includes management of the underlying data store and upload to GPU

    struct BufferBase {
        enum class BufferType {
            VertexBuffer, IndexBuffer
        };
        unsigned int id = 0;
        BufferType bufferType = BufferType::VertexBuffer;

        struct Layout {
            Layout(const std::string & name, SemanticType semanticType) : name(name), semanticType(semanticType) {}
            Layout(const Layout & rhs) : name(rhs.name), semanticType(rhs.semanticType) {}
            Layout & operator=(const Layout & rhs) { name = rhs.name; semanticType = rhs.semanticType; return * this; }

            std::string name;
            SemanticType semanticType;
        };

        std::vector<Layout> layout;

        LR_API BufferBase(BufferType bt) : id(0), bufferType(bt) {}
		LR_API virtual ~BufferBase();
		LR_API void bind() const;
		LR_API void unbind() const;

		LR_API void uploadStatic();
		LR_API void uploadDynamic();

        virtual void * buffer() const = 0;
        virtual size_t count() const = 0;
        virtual int stride() const = 0;

		LR_API virtual void setAttributes(VAO & vao);
    };

    // Buffer instantiates a backing store for BufferBase.
    // At the moment Buffer is memory backed, but it should optionally be GPU memory backed.

    template <typename T>
    class Buffer : public BufferBase {
        std::vector<T> _data;

    public:

        void push_back(T d) { _data.push_back(d); }

        T & elementAt(size_t i) {
            if (_data.size() > i)
                return _data[i];
            throw std::exception();     /// @TODO should be an error policy on BufferBase
        }

        virtual void * buffer() const override { return (void*) _data.data(); }

        Buffer(BufferBase::BufferType bt) : BufferBase(bt) {
            T::describeLayout(layout);
        }
        virtual ~Buffer() { }

        virtual int stride() const override { return sizeof(T); }
        virtual size_t count() const override { return _data.size(); }

        Buffer<T> &operator << (const T &t) { _data.push_back(t); return *this; }
    };

    // An index buffer is a convenience subclass filled wit IntEls. These are
    // analogous to the Vert structs below.

    struct IntEl {
        IntEl(int x) : x(x) {}
        IntEl(const IntEl& rhs) : x(rhs.x) { }
        static void describeLayout(std::vector<BufferBase::Layout> & layout) {
            layout.push_back(BufferBase::Layout("index", SemanticType::int_st));
        }
        int x;
    };

    class IndexBuffer : public Buffer<IntEl> {
    public:
        IndexBuffer() : Buffer<IntEl>(BufferType::IndexBuffer) {}
        virtual ~IndexBuffer() {}
        virtual void setAttributes(VAO& vao) override {}
    };

    // A VAO groups a vertex buffer and optionally indices together for rendering
    
    class VAO {
    protected:
        ErrorPolicy _errorPolicy;
        mutable unsigned int _id = 0;
        mutable int _offset = 0;
        int _stride = 0, _indexType = 0;
        mutable bool _needInit = true;
		mutable bool _indicesMustBeBound = true;

        std::shared_ptr<BufferBase> _vertices;   // vbo
        std::shared_ptr<IndexBuffer> _indices;   // ibo

    public:

        /// @TODO provide accessors for these two
        std::vector<Semantic> attributes;

		LR_API VAO(std::shared_ptr<BufferBase>, ErrorPolicy ep = ErrorPolicy::onErrorThrow);
		LR_API ~VAO();

        // pass in true if the data is going to be edited
        /// @TODO Should have a typename thing in Vertex, and should throw if a bad cast is being requested
        template <typename Vertex>
        Buffer<Vertex>* vertexData(bool edit) const {
            _needInit |= edit;
            return reinterpret_cast<Buffer<Vertex>*>(_vertices.get()); }

		LR_API bool hasAttribute(char const*const name) const;

        // Create a vertex array object referencing a shader and a vertex buffer.
        // The vertex buffer is used to determine the number of elements to
        // draw in draw() and drawInstanced().
        //
		LR_API VAO & setVertices(std::shared_ptr<BufferBase> vbo);
		LR_API VAO & setIndices(std::shared_ptr<IndexBuffer> ibo);
        
        // Define an attribute called name in the provided shader. This attribute
        // has count elements of type T. If normalized is true, integer types are
        // mapped from 0 to 2^n-1 (where n is the bit count) to values from 0 to 1.
        //
        // Attributes should be declared in the order they are declared in the
        // vertex struct (assuming interleaved data). Call check() after declaring
        // all attributes to make sure the vertex struct is packed.
        //
        // ex: struct VertPNT { vec3 pos; vec3 normal; vec3 uv; };
        //     attribute("pos", gl_vec3, 0).attribute("normal", gl_vec3, 1).attribute("uv", gl_vec2, 2);
        //

		LR_API VAO & attribute(const char *name, SemanticType t, int location, bool normalized = false);

        // Validate VBO modes and attribute byte sizes
		LR_API void check() const;

        // Draw the attached VBOs
		LR_API void draw() const;

        // Draw the attached VBOs using instancing
		LR_API void drawInstanced(int instances) const;
        
        // to be called when the data has been modified
		LR_API bool uploadVerts() const;
        
        LR_API void bindVAO() const;
        LR_API void unbindVAO() const;
    };

    // In the following structs, float[3] is used, not v3f, because v3f packs as v4f.

    struct VertP {
        VertP(v3f pos_) {
            pos[0] = pos_.x; pos[1] = pos_.y; pos[2] = pos_.z;
        }
        VertP(const VertP& rhs) {
            memcpy(pos, rhs.pos, sizeof(VertP));
        }
        static void describeLayout(std::vector<BufferBase::Layout> & layout) {
            layout.push_back(BufferBase::Layout("a_position", SemanticType::vec3_st));
        }
        float pos[3];
    };
    struct VertP2d {
        VertP2d(v2f pos_) {
            pos[0] = pos_.x; pos[1] = pos_.y;
        }
        VertP2d(float x, float y) { pos[0] = x; pos[1] = y; }
        VertP2d(const VertP2d& rhs) {
            memcpy(pos, rhs.pos, sizeof(VertP2d));
        }
        static void describeLayout(std::vector<BufferBase::Layout> & layout) {
            layout.push_back(BufferBase::Layout("a_position", SemanticType::vec2_st));
        }
        float pos[2];
    };
    struct VertPT {
        VertPT(v3f pos_, v2f uv_) {
            pos[0] = pos_.x; pos[1] = pos_.y; pos[2] = pos_.z;
            uv[0] = uv_.x; uv[1] = uv_.y;
        }
        VertPT(const VertPT& rhs) {
            memcpy(pos, rhs.pos, sizeof(VertPT));
        }
        static void describeLayout(std::vector<BufferBase::Layout> & layout) {
            layout.push_back(BufferBase::Layout("a_position", SemanticType::vec3_st));
            layout.push_back(BufferBase::Layout("a_uv", SemanticType::vec2_st));
        }
        float pos[3];
        float uv[2];
    };
    struct VertPN {
        VertPN(v3f pos_, v3f normal_) {
            pos[0] = pos_.x; pos[1] = pos_.y; pos[2] = pos_.z;
            normal[0] = normal_.x; normal[1] = normal_.y; normal[2] = normal_.z;
        }
        VertPN(const VertPN& rhs) {
            memcpy(pos, rhs.pos, sizeof(VertPN));
        }
        static void describeLayout(std::vector<BufferBase::Layout> & layout) {
            layout.push_back(BufferBase::Layout("a_position", SemanticType::vec3_st));
            layout.push_back(BufferBase::Layout("a_normal", SemanticType::vec3_st));
        }
        float pos[3];
        float normal[3];
    };
    struct VertPTN {
        VertPTN(v3f pos_, v2f uv_, v3f normal_) {
            pos[0] = pos_.x; pos[1] = pos_.y; pos[2] = pos_.z;
            normal[0] = normal_.x; normal[1] = normal_.y; normal[2] = normal_.z;
            uv[0] = uv_.x; uv[1] = uv_.y;
        }
        VertPTN(const VertPTN& rhs) {
            memcpy(pos, rhs.pos, sizeof(VertPTN));
        }
        static void describeLayout(std::vector<BufferBase::Layout> & layout) {
            layout.push_back(BufferBase::Layout("a_position", SemanticType::vec3_st));
            layout.push_back(BufferBase::Layout("a_normal", SemanticType::vec3_st));
            layout.push_back(BufferBase::Layout("a_uv", SemanticType::vec2_st));
        }
        float pos[3];
        float normal[3];
        float uv[2];
    };
    struct VertPT3N {
        VertPT3N(v3f pos_, v3f uvw_, v3f normal_) {
            pos[0] = pos_.x; pos[1] = pos_.y; pos[2] = pos_.z;
            normal[0] = normal_.x; normal[1] = normal_.y; normal[2] = normal_.z;
            uvw[0] = uvw_.x; uvw[1] = uvw_.y; uvw[2] = uvw_.z;
        }
        VertPT3N(const VertPT3N& rhs) {
            memcpy(pos, rhs.pos, sizeof(VertPT3N));
        }
        static void describeLayout(std::vector<BufferBase::Layout> & layout) {
            layout.push_back(BufferBase::Layout("a_position", SemanticType::vec3_st));
            layout.push_back(BufferBase::Layout("a_normal", SemanticType::vec3_st));
            layout.push_back(BufferBase::Layout("a_uvw", SemanticType::vec3_st));
        }
        float pos[3];
        float normal[3];
        float uvw[3];
    };
    struct VertPC {
        VertPC(v3f pos_, v4f color_) {
            pos[0] = pos_.x; pos[1] = pos_.y; pos[2] = pos_.z;
            color[0] = color_.x; color[1] = color_.y; color[2] = color_.z; color[3] = color_.w;
        }
        VertPC(const VertPN& rhs) {
            memcpy(pos, rhs.pos, sizeof(VertPC));
        }
        static void describeLayout(std::vector<BufferBase::Layout> & layout) {
            layout.push_back(BufferBase::Layout("a_position", SemanticType::vec3_st));
            layout.push_back(BufferBase::Layout("a_color", SemanticType::vec4_st));
        }
        float pos[3];
        float color[4];
    };
    struct VertPTC {
        VertPTC(v3f pos_, v2f uv_, v4f color_) {
            pos[0] = pos_.x; pos[1] = pos_.y; pos[2] = pos_.z;
            uv[0] = uv_.x; uv[1] = uv_.y;
            color[0] = color_.x; color[1] = color_.y; color[2] = color_.z; color[3] = color_.w;
        }
        VertPTC(const VertPN& rhs) {
            memcpy(pos, rhs.pos, sizeof(VertPTC));
        }
        static void describeLayout(std::vector<BufferBase::Layout> & layout) {
            layout.push_back(BufferBase::Layout("a_position", SemanticType::vec3_st));
            layout.push_back(BufferBase::Layout("a_color", SemanticType::vec4_st));
            layout.push_back(BufferBase::Layout("a_uv", SemanticType::vec2_st));
        }
        float pos[3];
        float color[4];
        float uv[2];
    };
    struct VertPTNC {
        VertPTNC(v3f pos_, v2f uv_, v3f normal_, v4f color_) {
            pos[0] = pos_.x; pos[1] = pos_.y; pos[2] = pos_.z;
            uv[0] = uv_.x; uv[1] = uv_.y;
            normal[0] = normal_.x; normal[1] = normal_.y; normal[2] = normal_.z;
            color[0] = color_.x; color[1] = color_.y; color[2] = color_.z; color[3] = color_.w;
        }
        VertPTNC(const VertPN& rhs) {
            memcpy(pos, rhs.pos, sizeof(VertPTC));
        }
        static void describeLayout(std::vector<BufferBase::Layout> & layout) {
            layout.push_back(BufferBase::Layout("a_position", SemanticType::vec3_st));
            layout.push_back(BufferBase::Layout("a_uv", SemanticType::vec2_st));
            layout.push_back(BufferBase::Layout("a_normal", SemanticType::vec3_st));
            layout.push_back(BufferBase::Layout("a_color", SemanticType::vec4_st));
        }
        float pos[3];
        float uv[2];
        float normal[3];
        float color[4];
    };

} // Lab
