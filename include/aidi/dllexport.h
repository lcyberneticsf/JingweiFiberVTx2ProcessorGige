#ifndef DLLEXPORT_H
#define DLLEXPORT_H

///////////////////////////////////////////////////////////
/// @author yupei wu
/// @brief export class tool
/// @date 2017/9/30
/////////////////////////////////////////////////////////

//#ifdef WIN64
//#define DLL_EXPORT __declspec(dllexport)
//#else
//#define DLL_EXPORT __attribute__ ((visibility ("default")))
//#endif
#ifdef _MSC_VER
#define DLL_EXPORT __declspec(dllimport)
#else
#define DLL_EXPORT 
#endif

#endif // DLLEXPORT_H
