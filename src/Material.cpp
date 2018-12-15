//
//  Material.cpp
//  LabApp
//
//  Created by Nick Porcino on 2014 03/5.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#include "LabRender/Material.h"
#include <unordered_map>

using namespace std;
namespace lab { namespace Render {

    class Material::Detail 
	{
    public:
        Detail() { }

        unordered_map<std::string, shared_ptr<InOut>> inlets;
    };

    Material::Material()
    : _detail(new Detail()) { }

    Material::~Material() {
        delete _detail;
    }

    void Material::setPropertyInlet(const std::string & name, std::shared_ptr<InOut> & inlet) {
        _detail->inlets[name] = inlet;
    }
    void Material::setPropertyInlet(char const*const name, std::shared_ptr<InOut> & inlet) {
        _detail->inlets[name] = inlet;
    }

    std::shared_ptr<InOut> Material::propertyInlet(const std::string & name) const {
        auto find = _detail->inlets.find(name);
        if (find == _detail->inlets.end())
            return shared_ptr<InOut>();
        return find->second;
    }
    
    std::shared_ptr<InOut> Material::propertyInlet(char const*const name) const {
        auto find = _detail->inlets.find(name);
        if (find == _detail->inlets.end())
            return shared_ptr<InOut>();
        return find->second;
    }

}}
