//
//  Outlet.h
//  LabApp
//
//  Created by Nick Porcino on 2014 03/5.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#pragma once

#include <memory>
#include <string>
#include <typeindex>

class InOut {
public:

    // Base for the templated Outlet, containing only the type_index of the
    // contained data.

    class OutletBase {
    public:
        OutletBase() : _type(typeid(OutletBase)) {}
        virtual ~OutletBase() {}
        const std::type_index & type() const { return _type; }

    protected:
        std::type_index _type;
    };


    // Outlet holds data

    template <typename T>
    class Outlet : public OutletBase {
    public:
        Outlet() { _type = typeid(T); }
        Outlet(const T & data) : _data(data) { _type = typeid(T); }
        virtual ~Outlet() {}
        const T & value() const { return _data; }
        void setValue(const T & i) { _data = i; }

    private:
        T _data;
    };

    InOut() {}
    InOut(std::shared_ptr<OutletBase> d) : _out(d), _outPtr(d.get()) {}
    virtual ~InOut() {}

    template <typename T>
    static std::shared_ptr<InOut> createInOut()            { return std::make_shared<InOut>( std::unique_ptr<Outlet<T>>(new Outlet<T>())); }
    template <typename T>
    static std::shared_ptr<InOut> createInOut(T & v)       { return std::make_shared<InOut>( std::unique_ptr<Outlet<T>>(new Outlet<T>(v))); }
    template <typename T>
    static std::shared_ptr<InOut> createInOut(const T & v) { return std::make_shared<InOut>( std::unique_ptr<Outlet<T>>(new Outlet<T>(v))); }

    template <typename T> const Outlet<T> * out() const { return dynamic_cast<Outlet<T>*>(_outPtr); }
    template <typename T>       Outlet<T> * out()       { return dynamic_cast<Outlet<T>*>(_outPtr); }

    template <typename T> void setValue(const T & i) const { (dynamic_cast<Outlet<T>*>(_outPtr))->setValue(i); }
    template <typename T> const T & selfValue()      const { return dynamic_cast<Outlet<T>*>(_outPtr)->value(); }
    template <typename T> const T & value() const
    {
        if (!_override.expired()) {
            auto p = _override.lock(); // lock is threadsafe
            auto o = dynamic_cast<Outlet<T>*>(p.get());
            if (!!o)
                return o->value();
        }
        return selfValue<T>();
    }

    bool inputConnected() const { return !_override.expired(); }
    std::shared_ptr<InOut> connectedInput() const { return _override.lock(); }
    void setOut(std::shared_ptr<InOut> & io) { _out = io->_out; _outPtr = _out.get(); }
    void setOut(std::shared_ptr<OutletBase> & out) { _out = out; _outPtr = _out.get(); }

    virtual bool connect(std::shared_ptr<InOut> & in)
    {
        if (!in) {
            _override.reset();
            return true;
        }
        if (_outPtr->type() != in->_outPtr->type()) {
            // failed to connect
            return false;
        }

        auto curr = _override;
        while (!curr.expired()) {
            auto j = curr.lock();
            if (j.get() == this) {
                // cycle detected, failed to connect
                return false;
            }
            curr = j->_override;
        }
        _override = in;
        return true;
    }
    void disconnect() { _override.reset(); }

protected:
    std::shared_ptr<OutletBase> _out;
    OutletBase * _outPtr;

    std::weak_ptr<InOut> _override;
};
