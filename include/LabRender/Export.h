
#pragma once

#if defined(_MSC_VER) && defined(LABRENDER_SHARED)
# ifdef BUILDING_LabRender
#  define LR_CAPI extern "C" __declspec(dllexport)
#  define LR_API __declspec(dllexport)
#  define LR_CLASS __declspec(dllexport)
# else
#  define LR_CAPI extern "C" __declspec(dllimport)
#  define LR_API __declspec(dllimport)
#  define LR_CLASS __declspec(dllimport)
# endif
#else
# define LR_API
# define LR_CAPI
# define LR_CLASS
#endif
