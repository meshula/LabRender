//
//  SemanticType.h
//  LabApp
//
//  Created by Nick Porcino on 2014 02/28.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#pragma once

#include <LabRender/LabRender.h>
#include <LabRenderTypes/SemanticType.h>
#include <set>
#include <string>
#include <vector>

namespace lab { namespace Render {


    bool semanticTypeIsSampler(SemanticType t);
	int semanticTypeToOpenGLElementType(SemanticType t);
	int semanticTypeElementCount(SemanticType t);
	int semanticTypeStride(SemanticType t);
	const char* semanticTypeName(SemanticType t);
	SemanticType semanticTypeNamed(const char*);
	std::string semanticTypeToString(SemanticType st);


}} //lab::Render
