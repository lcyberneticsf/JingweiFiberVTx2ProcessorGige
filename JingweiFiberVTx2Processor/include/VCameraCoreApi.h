#ifndef VSEE_CAMERA_CORE_API_H_
#define VSEE_CAMERA_CORE_API_H_

#ifdef _MSC_VER
#	ifdef VCameraCoreLib
#		define VCameraCoreApi __declspec(dllexport)
#	else
#		define VCameraCoreApi __declspec(dllimport)
#	endif
#else
#	define VCameraCoreApi
#endif

#ifdef VCameraCoreInside
#	undef VCameraCoreApi
#	define VCameraCoreApi
#endif


#endif