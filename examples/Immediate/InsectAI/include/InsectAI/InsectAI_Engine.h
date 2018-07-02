
// Copyright (c) 2003-2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

/** @file	Engine.h
	@brief	The AI simulator
*/

#ifndef _INSECTAI_ENGINE_H_
#define _INSECTAI_ENGINE_H_

#include "InsectAI/InsectAI.h"
#include "InsectAI/InsectAI_Agent.h"

#include <vector>

namespace InsectAI {


    ///////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class	Engine
    /// @brief	The AI simulator
    ///
	class Engine 
	{
	public:
		Engine();
		~Engine();

		/// Add an Entity to the simulation
		/// @return the ID of the Entity
		void	AddEntity(Entity* pEntity);

		void	RemoveEntity(Entity* p);
		void	RemoveAllEntities();
		void	UpdateEntities(float epoch, float dt);
        
        Entity* FindClosestEntity(Entity*, uint32_t kind);
        void    FindClosestKEntities(const size_t k, Entity* pEntity, uint32_t kind, std::vector<Entity*>& result);
        
    private:
        class Detail;
        Detail* _detail;
	};

}	// end namespace InsectAI

#endif
