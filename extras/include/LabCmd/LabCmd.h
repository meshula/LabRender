#pragma once

#if defined(_MSC_VER) && defined(LABCMD_DLL)
# ifdef BUILDING_LABCMD
#  define LC_CAPI extern "C" __declspec(dllexport)
#  define LC_API __declspec(dllexport)
#  define LC_CLASS __declspec(dllexport)
# else
#  define LC_CAPI extern "C" __declspec(dllimport)
#  define LC_API __declspec(dllimport)
#  define LC_CLASS __declspec(dllimport)
# endif
#else
# define LC_API
# define LC_CAPI
# define LC_CLASS
#endif
