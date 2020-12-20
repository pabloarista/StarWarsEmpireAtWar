#ifndef PDLL_H_INCLUDED
#define PDLL_H_INCLUDED

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif

#endif // PDLL_H_INCLUDED
