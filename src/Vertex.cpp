
//
//  Vertex.cpp
//  LabApp
//
//  Created by Nick Porcino on 2014 03/7.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#include "LabRender/Vertex.h"
#include "LabRender/gl4.h"


namespace lab {
    
    void BufferBase::setAttributes(VAO & vao) {
        int i = 0;
        for (auto l : layout)
            vao.attribute(l.name.c_str(), l.semanticType, i++);
    }

    
    BufferBase::~BufferBase() { glDeleteBuffers(1, &id); }
    void BufferBase::bind() const   { glBindBuffer(bufferType == BufferType::VertexBuffer? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER, id); }
    void BufferBase::unbind() const { glBindBuffer(bufferType == BufferType::VertexBuffer? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER, 0); }
    
    void BufferBase::uploadDynamic() {
        if (!id) {
            glGenBuffers(1, &id); }
        bind();
        glBufferData(bufferType == BufferType::VertexBuffer? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER,
                     count() * stride(), buffer(), GL_DYNAMIC_DRAW);
        unbind();
    }

    void BufferBase::uploadStatic() {
        if (!id) {
            glGenBuffers(1, &id); }
        bind();
        glBufferData(bufferType == BufferType::VertexBuffer? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER,
                     count() * stride(), buffer(), GL_STATIC_DRAW);
        unbind();
    }
    

    VAO::VAO(std::shared_ptr<BufferBase> verts, ErrorPolicy ep)
    : _vertices(verts), _errorPolicy(ep), _id(0), _stride(0), _offset(0), _indexType(GL_INVALID_ENUM), _needInit(true) {
    }

    VAO::~VAO() { glDeleteVertexArrays(1, &_id); }


    VAO & VAO::attribute(const char *name, SemanticType t, int location, bool normalized) {
        if (location >= 0) {
            _vertices->bind();
            bindVAO();
            glEnableVertexAttribArray(location);
            glVertexAttribPointer(location,
                                  semanticTypeElementCount(t),
                                  semanticTypeToOpenGLElementType(t),
                                  normalized,
                                  _stride, (char *)NULL + _offset);
            unbindVAO();
            _vertices->unbind();
            _offset += semanticTypeStride(t);

            attributes.resize(location + 1);
            attributes[location].name = std::string(name);
            attributes[location].type = t;
            attributes[location].location = location;
        }
        else {
            std::string err = "invalid location for attribute ";
            err += name;
            handleGLError(_errorPolicy, GL_INVALID_VALUE, err.c_str());
        }
        return *this;
    }

    bool VAO::hasAttribute(char const*const name) const {
        for (auto n : attributes)
            if (!strcmp(name, n.name.c_str()))
                return true;

        return false;
    }


    void VAO::bindVAO() const {
        glBindVertexArray(_id);
        checkError(_errorPolicy, TestConditions::exhaustive, "VAO::bindVAO");
    }
    void VAO::unbindVAO() const { glBindVertexArray(0); }

    bool VAO::uploadVerts() const
    {
		if (_indicesMustBeBound) {
			if (_indices && _indexType != GL_INVALID_VALUE) {
				bindVAO();
				_indices->bind();
				unbindVAO();
				_indices->unbind();
			}
			else {
				bindVAO();
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				unbindVAO();
			}
			_indicesMustBeBound = false;
		}
		if (_needInit) {
            try {
                VAO* self = const_cast<VAO*>(this);
                _vertices->uploadStatic();
                self->setVertices(_vertices);
                if (!!_indices) {
                    _indices->uploadStatic();
                    self->setIndices(_indices);
                }
                _offset = 0;
                _vertices->setAttributes(*self);
                check();
                _needInit = false;
            }
            catch(std::exception& exc) {
                std::cout << exc.what() << std::endl;
            }
        }
        return !_needInit;
    }

    void VAO::draw() const {
        checkError(_errorPolicy, TestConditions::exhaustive, "VAO::draw start");
        uploadVerts();
        if (_indices) {
            bindVAO();
            glDrawRangeElements(GL_TRIANGLES, 0, (int) _indices->count(), (int) _indices->count(), _indexType, NULL);
            //glDrawElements(mode, _indices->size(), _indexType, NULL);
            unbindVAO();
        }
        else if (_vertices) {
            bindVAO();
            glDrawArrays(GL_TRIANGLES, 0, (int) _vertices->count());
            checkError(_errorPolicy, TestConditions::exhaustive, "VAO::drawArrays");
            unbindVAO();
        }
    }
    
    void VAO::drawInstanced(int instances) const {
        bindVAO();
        if (_indices)
            glDrawElementsInstanced(GL_TRIANGLES, (int) _indices->count(), _indexType, NULL, instances);
        else
            glDrawArraysInstanced(GL_TRIANGLES, 0, (int) _vertices->count(), instances);
        unbindVAO();
    }


    void VAO::check() const {
        if (_vertices && _vertices->bufferType != BufferBase::BufferType::VertexBuffer) {
            handleGLError(_errorPolicy, GL_INVALID_OPERATION, "expected vertices to have type GL_ARRAY_BUFFER");
        }
        else if (_indices && _indices->bufferType != BufferBase::BufferType::IndexBuffer) {
            handleGLError(_errorPolicy, GL_INVALID_OPERATION, "expected indices to have type GL_ELEMENT_ARRAY_BUFFER");
        }
        else if (_offset != _stride) {
            handleGLError(_errorPolicy, GL_INVALID_OPERATION, "expected size of attributes to add up to size of vertex");
        }
        checkError(_errorPolicy, TestConditions::exhaustive, "VAO::check");
    }
    
    VAO & VAO::setVertices(std::shared_ptr<BufferBase> vbo) {
        _vertices = vbo;
        _stride = vbo->stride();
        
        if (!_id)
            glGenVertexArrays(1, &_id);
        
        bindVAO();
        vbo->bind();
        unbindVAO();
        vbo->unbind();
        return *this;
    }
    
    VAO & VAO::setIndices(std::shared_ptr<IndexBuffer> ibo) {
        if (ibo) {
            _indexType = TypeToOpenGL<unsigned int>::value;
        }
        else {
            _indexType = GL_INVALID_VALUE;
        }
        _indices = ibo;
		_indicesMustBeBound = true;
        return *this;
    }

    
} // LabRender
