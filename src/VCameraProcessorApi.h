#ifndef VSEE_CAMERA_PROCESSOR_API_H_
#define VSEE_CAMERA_PROCESSOR_API_H_

#ifdef _MSC_VER
	#ifdef VCameraProcessorLib
		#define VCameraProcessorApi __declspec(dllexport)
	#else
		#define VCameraProcessorApi __declspec(dllimport)
	#endif
#else
	#define VCameraProcessorApi
#endif

#ifdef VCameraProcessorInside
	#undef VCameraProcessorApi
	#define VCameraProcessorApi
#endif

#endif