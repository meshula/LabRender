
#pragma once

#include <LabRender/LabRender.h>
#include <LabRender/Model.h>
#include <memory>
#include <string>

#if defined(_MSC_VER) && defined(LABRENDER_MODELLOADER_DLL)
# ifdef BUILDING_LABRENDER_MODELLOADER
#  define LRML_CAPI extern "C" __declspec(dllexport)
#  define LRML_API __declspec(dllexport)
#  define LRML_CLASS __declspec(dllexport)
# else
#  define LRML_CAPI extern "C" __declspec(dllimport)
#  define LRML_API __declspec(dllimport)
#  define LRML_CLASS __declspec(dllimport)
# endif
#else
# define LRML_API
# define LRML_CAPI
# define LRML_CLASS
#endif

namespace lab 
{
	LRML_API std::shared_ptr<Model> loadMesh(const std::string& filename);
}
