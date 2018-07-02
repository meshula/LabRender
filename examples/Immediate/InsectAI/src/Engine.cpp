
// Copyright (c) 2003-2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "InsectAI/InsectAI.h"
#include "InsectAI/InsectAI_Engine.h"
#include "InsectAI/InsectAI_Agent.h"

#include "InsectAI/nanoflann.hpp"
#include <map>

namespace InsectAI {

    using Lab::LabVec3;
    using namespace nanoflann;
    
    template <class T>
    struct EntityMap
    {        
        std::vector<Entity*> pts;

        void addEntity(Entity* pEntity)
        {
            bool found = false;
            for (auto i = pts.begin(); i != pts.end(); ++i)
                if (*i == pEntity) {
                    found = true;
                    break;
                }
            
            if (!found)
                pts.push_back(pEntity);
        }

        void removeEntity(Entity* pEntity)
        {
            for (auto i = pts.begin(); i != pts.end(); ++i)
                if (*i == pEntity) {
                    pts.erase(i);
                    break;
                }
        }
                
        // Must return the number of data points
        inline size_t kdtree_get_point_count() const { return pts.size(); }
        
        // Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
        inline T kdtree_distance(const T *p1, const size_t idx_p2,size_t size) const
        {
            LabVec3 v = pts[idx_p2]->state().position;
            const T d0=p1[0] - v[0];
            const T d1=p1[1] - v[1];
            const T d2=p1[2] - v[2];
            return d0*d0+d1*d1+d2*d2;
        }
        
        // Returns the dim'th component of the idx'th point in the class:
        // Since this is inlined and the "dim" argument is typically an immediate value, the
        //  "if/else's" are actually solved at compile time.
        inline T kdtree_get_pt(const size_t idx, int dim) const
        {
            //if (idx > pts.size())
            //    RaiseError(0, "baz", "baz");
            //if (!pts[idx])
            //    RaiseError(0, "foo", "foo");
            Lab::DynamicState& ds = pts[idx]->state();
            //if (!ds)
            //    RaiseError(0, "bar","bar");
            LabVec3 v = ds.position;
            return v[dim];
        }
        
        // Optional bounding-box computation: return false to default to a standard bbox computation loop.
        //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
        //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
        template <class BBOX>
        bool kdtree_get_bbox(BBOX &bb) const { return false; }
    };
    
    typedef float num_t;
    class Engine::Detail
    {
    public:
        // construct a kd-tree index:
        typedef KDTreeSingleIndexAdaptor< L2_Simple_Adaptor<num_t, EntityMap<num_t> > ,
                                            EntityMap<num_t>,
                                            3 /* dim */ > my_kd_tree_t;

        Detail()
        {
            lightsIndex = new my_kd_tree_t(3 /*dim*/, lights, KDTreeSingleIndexAdaptorParams(10 /* max leaf */) );
            entitiesIndex = new my_kd_tree_t(3 /*dim*/, entities, KDTreeSingleIndexAdaptorParams(10 /* max leaf */) );
        }
        
        ~Detail()
        {
            delete lightsIndex;
            delete entitiesIndex;
        }
        
        void update()
        {
            if (lights.pts.size() > 0)
                lightsIndex->buildIndex();
            if (entities.pts.size() > 0)
                entitiesIndex->buildIndex();
        }

        Entity* findClosestEntity(Entity* pEntity, uint32_t kind)
        {
            Lab::DynamicState& ds = pEntity->state();
            LabVec3 pos = ds.position;
            num_t query_pt[3];
            for (int i = 0; i < 3; ++i)
                query_pt[i] = pos[i];
            
            if (kind == kKindLight) {
                const size_t num_results = 1;
                std::vector<size_t>   ret_index(num_results);
                std::vector<num_t> out_dist_sqr(num_results);
                lightsIndex->knnSearch(&query_pt[0], num_results, &ret_index[0], &out_dist_sqr[0]);
                if (ret_index.size()) {
                    size_t i = ret_index[0];
                    if (i < lights.pts.size())
                        return lights.pts[i];
                    else
                        return 0;
                }
                else
                    return 0;
            }

            const size_t num_results = 1;
            std::vector<size_t> ret_index(num_results);
            std::vector<num_t> out_dist_sqr(num_results);
            entitiesIndex->knnSearch(&query_pt[0], num_results, &ret_index[0], &out_dist_sqr[0]);
            if (ret_index.size()) {
                size_t i = ret_index[0];
                if (i < entities.pts.size())
                    return entities.pts[i];
                else
                    return 0;
            }
            else
                return 0;
        }
        
        void findClosestKEntities(const size_t k, Entity* pEntity, uint32_t kind, std::vector<Entity*>& result)
        {
            result.resize(k);
            
            Lab::DynamicState& ds = pEntity->state();
            LabVec3 pos = ds.position;
            num_t query_pt[3];
            for (int i = 0; i < 3; ++i)
                query_pt[i] = pos[i];

            if (kind == kKindLight) {
                std::vector<size_t>   ret_index(k);
                std::vector<num_t> out_dist_sqr(k);
                lightsIndex->knnSearch(&query_pt[0], k, &ret_index[0], &out_dist_sqr[0]);
                
                for (int i = 0; i < k; ++i)
                    result[i] = lights.pts[ret_index[i]];
                
                return;
            }
            
            std::vector<size_t> ret_index(k);
            std::vector<num_t> out_dist_sqr(k);
            entitiesIndex->knnSearch(&query_pt[0], k, &ret_index[0], &out_dist_sqr[0]);

            for (int i = 0; i < k; ++i)
                result[i] = entities.pts[ret_index[i]];
            
            return;
        }
        
        my_kd_tree_t*   lightsIndex;
        my_kd_tree_t*   entitiesIndex;
        
        EntityMap<num_t> lights;
		EntityMap<num_t> entities;
    };
    
    Engine::Engine()
    : _detail(new Detail())
    {
    }

    Engine::~Engine()
    {
        RemoveAllEntities();
        delete _detail;
    }

    void Engine::AddEntity(Entity* pEntity)
    {
        if (pEntity->GetKind() == kKindLight)
            _detail->lights.addEntity(pEntity);
        else
            _detail->entities.addEntity(pEntity);
    }

    void Engine::RemoveEntity(Entity* pEntity)
    {
        _detail->lights.removeEntity(pEntity);
        _detail->entities.removeEntity(pEntity);
    }

    void Engine::RemoveAllEntities()
    {
        _detail->lights.pts.clear();
        _detail->entities.pts.clear();
    }

    void Engine::UpdateEntities(float epoch, float dt)
    {
        _detail->update();
        for (auto i = _detail->lights.pts.begin(); i != _detail->lights.pts.end(); ++i)
            (*i)->Update(this, epoch, dt);
        for (auto i = _detail->entities.pts.begin(); i != _detail->entities.pts.end(); ++i)
            (*i)->Update(this, epoch, dt);
    }
    
    Entity* Engine::FindClosestEntity(Entity* entity, uint32_t kind)
    {
        return _detail->findClosestEntity(entity, kind);
    }
    
    void Engine::FindClosestKEntities(const size_t k, Entity* pEntity, uint32_t kind, std::vector<Entity*>& result)
    {
        return _detail->findClosestKEntities(k, pEntity, kind, result);
    }



} // InsectAI
